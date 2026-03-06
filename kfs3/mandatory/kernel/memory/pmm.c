/*
 * Physical Memory Manager (PMM)
 * Uses a bitmap to track free/used physical pages
 */

#include "memory.h"
#include "screen.h"
#include "panic.h"

/* Bitmap for tracking physical pages */
static uint32_t *pmm_bitmap = NULL;
static uint32_t pmm_bitmap_size = 0;      /* Size in uint32_t entries */
static uint32_t pmm_total_pages = 0;
static uint32_t pmm_used_pages = 0;
static uint32_t pmm_total_memory = 0;      /* Total memory in bytes */

/* Bitmap manipulation macros */
#define BITMAP_INDEX(page)  ((page) / 32)
#define BITMAP_OFFSET(page) ((page) % 32)

/* Set a bit (mark page as used) */
static inline void bitmap_set(uint32_t page)
{
    pmm_bitmap[BITMAP_INDEX(page)] |= (1 << BITMAP_OFFSET(page));
}

/* Clear a bit (mark page as free) */
static inline void bitmap_clear(uint32_t page)
{
    pmm_bitmap[BITMAP_INDEX(page)] &= ~(1 << BITMAP_OFFSET(page));
}

/* Test a bit (check if page is used) */
static inline int bitmap_test(uint32_t page)
{
    return (pmm_bitmap[BITMAP_INDEX(page)] & (1 << BITMAP_OFFSET(page))) != 0;
}

/* Find first free page */
static uint32_t pmm_find_free_page(void)
{
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            /* Found a uint32_t with at least one free bit */
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t page = i * 32 + j;
                if (page < pmm_total_pages && !bitmap_test(page)) {
                    return page;
                }
            }
        }
    }
    return (uint32_t)-1;  /* No free pages */
}

/* Initialize physical memory manager from multiboot info */
void pmm_init(multiboot_info_t *mboot)
{
    if (!(mboot->flags & MULTIBOOT_FLAG_MEM)) {
        PANIC("Multiboot memory info not available");
    }
    
    /* Calculate total memory (mem_upper is in KB, starts at 1MB) */
    pmm_total_memory = (mboot->mem_upper + 1024) * 1024;
    pmm_total_pages = pmm_total_memory / PAGE_SIZE;
    
    /* Calculate bitmap size */
    pmm_bitmap_size = (pmm_total_pages + 31) / 32;  /* Round up */
    
    /* Place bitmap right after kernel end, aligned to page boundary */
    uint32_t kernel_end_addr = (uint32_t)&kernel_end;
    kernel_end_addr = (kernel_end_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    pmm_bitmap = (uint32_t*)kernel_end_addr;
    
    /* Initially mark all pages as used */
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }
    pmm_used_pages = pmm_total_pages;
    
    /* Parse memory map if available */
    if (mboot->flags & MULTIBOOT_FLAG_MMAP) {
        multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t*)mboot->mmap_addr;
        uint32_t mmap_end = mboot->mmap_addr + mboot->mmap_length;
        
        while ((uint32_t)mmap < mmap_end) {
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                /* Mark available memory regions as free */
                uint64_t start = mmap->addr;
                uint64_t end = start + mmap->len;
                
                /* Align to page boundaries */
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
            
            /* Move to next entry */
            mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    } else {
        /* No memory map, assume everything above 1MB is free */
        for (uint32_t page = 256; page < pmm_total_pages; page++) {  /* 256 * 4KB = 1MB */
            bitmap_clear(page);
            pmm_used_pages--;
        }
    }
    
    /* Mark kernel area as used (including bitmap) */
    uint32_t kernel_start_page = (uint32_t)&kernel_start / PAGE_SIZE;
    uint32_t bitmap_end = kernel_end_addr + pmm_bitmap_size * sizeof(uint32_t);
    uint32_t kernel_end_page = (bitmap_end + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t page = kernel_start_page; page < kernel_end_page; page++) {
        if (!bitmap_test(page)) {
            bitmap_set(page);
            pmm_used_pages++;
        }
    }
    
    /* Mark first 1MB as used (BIOS, VGA, etc.) */
    for (uint32_t page = 0; page < 256; page++) {
        if (!bitmap_test(page)) {
            bitmap_set(page);
            pmm_used_pages++;
        }
    }
}

/* Allocate a physical page */
void* pmm_alloc_page(void)
{
    uint32_t page = pmm_find_free_page();
    
    if (page == (uint32_t)-1) {
        kernel_warning("PMM: Out of memory!", __FILE__, __LINE__);
        return NULL;
    }
    
    bitmap_set(page);
    pmm_used_pages++;
    
    /* Return physical address */
    void *addr = (void*)(page * PAGE_SIZE);
    
    /* Zero the page */
    memset(addr, 0, PAGE_SIZE);
    
    return addr;
}

/* Free a physical page */
void pmm_free_page(void *page)
{
    if (page == NULL) {
        return;
    }
    
    uint32_t page_num = (uint32_t)page / PAGE_SIZE;
    
    if (page_num >= pmm_total_pages) {
        kernel_warning("PMM: Attempt to free invalid page", __FILE__, __LINE__);
        return;
    }
    
    if (!bitmap_test(page_num)) {
        kernel_warning("PMM: Double free detected", __FILE__, __LINE__);
        return;
    }
    
    bitmap_clear(page_num);
    pmm_used_pages--;
}

/* Get total memory size in bytes */
uint32_t pmm_get_total_memory(void)
{
    return pmm_total_memory;
}

/* Get free memory size in bytes */
uint32_t pmm_get_free_memory(void)
{
    return (pmm_total_pages - pmm_used_pages) * PAGE_SIZE;
}

/* Get used memory size in bytes */
uint32_t pmm_get_used_memory(void)
{
    return pmm_used_pages * PAGE_SIZE;
}

/* Memory utility functions */
void* memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t*)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

void* memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t*)dest;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t*)s1;
    const uint8_t *p2 = (const uint8_t*)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}
