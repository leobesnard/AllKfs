/*
 * Physical Memory Manager (PMM) - Bonus Version
 * With additional debug and dump functions
 */

#include "memory.h"
#include "screen.h"
#include "panic.h"

static uint32_t *pmm_bitmap = NULL;
static uint32_t pmm_bitmap_size = 0;
static uint32_t pmm_total_pages = 0;
static uint32_t pmm_used_pages = 0;
static uint32_t pmm_total_memory = 0;

#define BITMAP_INDEX(page)  ((page) / 32)
#define BITMAP_OFFSET(page) ((page) % 32)

static inline void bitmap_set(uint32_t page)
{
    pmm_bitmap[BITMAP_INDEX(page)] |= (1 << BITMAP_OFFSET(page));
}

static inline void bitmap_clear(uint32_t page)
{
    pmm_bitmap[BITMAP_INDEX(page)] &= ~(1 << BITMAP_OFFSET(page));
}

static inline int bitmap_test(uint32_t page)
{
    return (pmm_bitmap[BITMAP_INDEX(page)] & (1 << BITMAP_OFFSET(page))) != 0;
}

static uint32_t pmm_find_free_page(void)
{
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t page = i * 32 + j;
                if (page < pmm_total_pages && !bitmap_test(page)) {
                    return page;
                }
            }
        }
    }
    return (uint32_t)-1;
}

void pmm_init(multiboot_info_t *mboot)
{
    if (!(mboot->flags & MULTIBOOT_FLAG_MEM)) {
        PANIC("Multiboot memory info not available");
    }
    
    pmm_total_memory = (mboot->mem_upper + 1024) * 1024;
    pmm_total_pages = pmm_total_memory / PAGE_SIZE;
    pmm_bitmap_size = (pmm_total_pages + 31) / 32;
    
    uint32_t kernel_end_addr = (uint32_t)&kernel_end;
    kernel_end_addr = (kernel_end_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    pmm_bitmap = (uint32_t*)kernel_end_addr;
    
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }
    pmm_used_pages = pmm_total_pages;
    
    if (mboot->flags & MULTIBOOT_FLAG_MMAP) {
        multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t*)mboot->mmap_addr;
        uint32_t mmap_end = mboot->mmap_addr + mboot->mmap_length;
        
        while ((uint32_t)mmap < mmap_end) {
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                uint64_t start = mmap->addr;
                uint64_t end = start + mmap->len;
                
                start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
                end = end & ~(PAGE_SIZE - 1);
                
                for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
                    if (addr < pmm_total_memory) {
                        uint32_t page = addr / PAGE_SIZE;
                        bitmap_clear(page);
                        pmm_used_pages--;
                    }
                }
            }
            mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    } else {
        for (uint32_t page = 256; page < pmm_total_pages; page++) {
            bitmap_clear(page);
            pmm_used_pages--;
        }
    }
    
    uint32_t kernel_start_page = (uint32_t)&kernel_start / PAGE_SIZE;
    uint32_t bitmap_end = kernel_end_addr + pmm_bitmap_size * sizeof(uint32_t);
    uint32_t kernel_end_page = (bitmap_end + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t page = kernel_start_page; page < kernel_end_page; page++) {
        if (!bitmap_test(page)) {
            bitmap_set(page);
            pmm_used_pages++;
        }
    }
    
    for (uint32_t page = 0; page < 256; page++) {
        if (!bitmap_test(page)) {
            bitmap_set(page);
            pmm_used_pages++;
        }
    }
}

void* pmm_alloc_page(void)
{
    uint32_t page = pmm_find_free_page();
    
    if (page == (uint32_t)-1) {
        kernel_warning("PMM: Out of memory!", __FILE__, __LINE__);
        return NULL;
    }
    
    bitmap_set(page);
    pmm_used_pages++;
    
    void *addr = (void*)(page * PAGE_SIZE);
    memset(addr, 0, PAGE_SIZE);
    
    return addr;
}

void pmm_free_page(void *page)
{
    if (page == NULL) return;
    
    uint32_t page_num = (uint32_t)page / PAGE_SIZE;
    
    if (page_num >= pmm_total_pages) {
        kernel_warning("PMM: Invalid page address", __FILE__, __LINE__);
        return;
    }
    
    if (!bitmap_test(page_num)) {
        kernel_warning("PMM: Double free detected", __FILE__, __LINE__);
        return;
    }
    
    bitmap_clear(page_num);
    pmm_used_pages--;
}

uint32_t pmm_get_total_memory(void) { return pmm_total_memory; }
uint32_t pmm_get_free_memory(void) { return (pmm_total_pages - pmm_used_pages) * PAGE_SIZE; }
uint32_t pmm_get_used_memory(void) { return pmm_used_pages * PAGE_SIZE; }
uint32_t pmm_get_total_pages(void) { return pmm_total_pages; }
uint32_t pmm_get_used_pages(void) { return pmm_used_pages; }

void* memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t*)s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

void* memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t*)dest;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t*)s1;
    const uint8_t *p2 = (const uint8_t*)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

/* Debug: Dump memory page bitmap status */
void memory_dump_page(uint32_t page_num)
{
    if (page_num >= pmm_total_pages) {
        print_str("Invalid page number", RED);
        print_new_line();
        return;
    }
    
    print_str("Page ", WHITE);
    print_dec(page_num, YELLOW);
    print_str(": ", WHITE);
    print_str(bitmap_test(page_num) ? "USED" : "FREE", bitmap_test(page_num) ? RED : GREEN);
    print_str("  Address: ", WHITE);
    print_hex(page_num * PAGE_SIZE, CYAN);
    print_new_line();
}
