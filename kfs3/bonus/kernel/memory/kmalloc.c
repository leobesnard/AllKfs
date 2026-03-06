/*
 * Kernel Memory Allocator - Bonus Version with Debug
 */

#include "memory.h"
#include "screen.h"
#include "panic.h"

typedef struct heap_block {
    uint32_t magic;
    size_t size;
    uint8_t free;
    struct heap_block *next;
    struct heap_block *prev;
} __attribute__((packed)) heap_block_t;

#define HEAP_MAGIC          0xDEADBEEF
#define HEAP_MAGIC_FREE     0xFEEDFACE
#define MIN_BLOCK_SIZE      16

static heap_block_t *heap_start = NULL;
static heap_block_t *heap_end = NULL;
static uint32_t heap_base = 0;
static uint32_t heap_current = 0;
static uint32_t heap_limit = 0;
static size_t heap_total_allocated = 0;
static size_t heap_total_free = 0;

static inline size_t align_size(size_t size)
{
    return (size + 3) & ~3;
}

void kheap_init(void)
{
    uint32_t kernel_end_addr = (uint32_t)&kernel_end;
    uint32_t pmm_bitmap_size = (pmm_get_total_memory() / PAGE_SIZE + 31) / 32 * sizeof(uint32_t);
    heap_base = (kernel_end_addr + pmm_bitmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    heap_limit = heap_base + (4 * 1024 * 1024);
    heap_current = heap_base;
    
    heap_start = (heap_block_t*)heap_base;
    heap_start->magic = HEAP_MAGIC_FREE;
    heap_start->size = heap_limit - heap_base - sizeof(heap_block_t);
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_end = heap_start;
    heap_total_free = heap_start->size;
    heap_total_allocated = 0;
}

static heap_block_t* find_free_block(size_t size)
{
    heap_block_t *block = heap_start;
    
    while (block) {
        if (block->free && block->size >= size) {
            if (block->magic != HEAP_MAGIC_FREE) {
                kernel_panic("Heap corruption", __FILE__, __LINE__, PANIC_HEAP_CORRUPTION);
            }
            return block;
        }
        block = block->next;
    }
    return NULL;
}

static void split_block(heap_block_t *block, size_t size)
{
    if (block->size <= size + sizeof(heap_block_t) + MIN_BLOCK_SIZE) return;
    
    heap_block_t *new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
    new_block->magic = HEAP_MAGIC_FREE;
    new_block->size = block->size - size - sizeof(heap_block_t);
    new_block->free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) block->next->prev = new_block;
    block->next = new_block;
    block->size = size;
    
    if (heap_end == block) heap_end = new_block;
    heap_total_free += new_block->size;
}

void* kmalloc(size_t size)
{
    if (size == 0) return NULL;
    
    size = align_size(size);
    
    heap_block_t *block = find_free_block(size);
    if (block == NULL) {
        kernel_warning("kmalloc: Out of memory", __FILE__, __LINE__);
        return NULL;
    }
    
    split_block(block, size);
    block->magic = HEAP_MAGIC;
    block->free = 0;
    
    heap_total_allocated += block->size;
    heap_total_free -= block->size;
    
    return (void*)((uint8_t*)block + sizeof(heap_block_t));
}

void* kmalloc_aligned(size_t size, size_t alignment)
{
    if (size == 0 || alignment == 0) return NULL;
    
    size_t total_size = size + alignment + sizeof(uint32_t);
    void *ptr = kmalloc(total_size);
    if (ptr == NULL) return NULL;
    
    uint32_t addr = (uint32_t)ptr + sizeof(uint32_t);
    uint32_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
    
    *((uint32_t*)(aligned_addr - sizeof(uint32_t))) = aligned_addr - (uint32_t)ptr;
    
    return (void*)aligned_addr;
}

static void merge_blocks(heap_block_t *block)
{
    while (block->next && block->next->free) {
        heap_total_free -= block->next->size;
        block->size += sizeof(heap_block_t) + block->next->size;
        
        if (heap_end == block->next) heap_end = block;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    
    if (block->prev && block->prev->free) {
        heap_total_free -= block->prev->size;
        block->prev->size += sizeof(heap_block_t) + block->size;
        
        if (heap_end == block) heap_end = block->prev;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        
        block = block->prev;
    }
    
    heap_total_free += block->size;
}

void kfree(void *ptr)
{
    if (ptr == NULL) return;
    
    heap_block_t *block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    
    if (block->magic != HEAP_MAGIC) {
        if (block->magic == HEAP_MAGIC_FREE) {
            kernel_panic("Double free", __FILE__, __LINE__, PANIC_DOUBLE_FREE);
        } else {
            kernel_panic("Invalid pointer", __FILE__, __LINE__, PANIC_INVALID_POINTER);
        }
        return;
    }
    
    block->magic = HEAP_MAGIC_FREE;
    block->free = 1;
    
    heap_total_allocated -= block->size;
    merge_blocks(block);
}

size_t ksize(void *ptr)
{
    if (ptr == NULL) return 0;
    
    heap_block_t *block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) return 0;
    
    return block->size;
}

void* kbrk(intptr_t increment)
{
    if (increment == 0) return (void*)heap_current;
    
    uint32_t old_brk = heap_current;
    uint32_t new_brk = heap_current + increment;
    
    if (new_brk < heap_base || new_brk > heap_limit) {
        kernel_warning("kbrk: Out of bounds", __FILE__, __LINE__);
        return (void*)-1;
    }
    
    heap_current = new_brk;
    return (void*)old_brk;
}

void kheap_print_info(void)
{
    print_str("=== Kernel Heap ===", CYAN);
    print_new_line();
    print_str("Base:      ", WHITE);
    print_hex(heap_base, YELLOW);
    print_new_line();
    print_str("Limit:     ", WHITE);
    print_hex(heap_limit, YELLOW);
    print_new_line();
    print_str("Total:     ", WHITE);
    print_dec((heap_limit - heap_base) / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    print_str("Allocated: ", WHITE);
    print_dec(heap_total_allocated / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    print_str("Free:      ", WHITE);
    print_dec(heap_total_free / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    
    /* Count blocks */
    int total_blocks = 0, free_blocks = 0;
    heap_block_t *block = heap_start;
    while (block) {
        total_blocks++;
        if (block->free) free_blocks++;
        block = block->next;
    }
    
    print_str("Blocks:    ", WHITE);
    print_dec(total_blocks, YELLOW);
    print_str(" (", LIGHT_GREY);
    print_dec(free_blocks, GREEN);
    print_str(" free)", LIGHT_GREY);
    print_new_line();
}

void memory_print_info(void)
{
    print_str("========================================", WHITE);
    print_new_line();
    print_str("       MEMORY INFORMATION", CYAN);
    print_new_line();
    print_str("========================================", WHITE);
    print_new_line();
    print_new_line();
    
    print_str("=== Physical Memory ===", CYAN);
    print_new_line();
    print_str("Total:  ", WHITE);
    print_dec(pmm_get_total_memory() / 1024 / 1024, YELLOW);
    print_str(" MB (", LIGHT_GREY);
    print_dec(pmm_get_total_pages(), WHITE);
    print_str(" pages)", LIGHT_GREY);
    print_new_line();
    print_str("Used:   ", WHITE);
    print_dec(pmm_get_used_memory() / 1024, YELLOW);
    print_str(" KB (", LIGHT_GREY);
    print_dec(pmm_get_used_pages(), WHITE);
    print_str(" pages)", LIGHT_GREY);
    print_new_line();
    print_str("Free:   ", WHITE);
    print_dec(pmm_get_free_memory() / 1024 / 1024, YELLOW);
    print_str(" MB", WHITE);
    print_new_line();
    print_new_line();
    
    kheap_print_info();
}

/* Memory dump function for bonus */
void memory_dump(uint32_t address, uint32_t size)
{
    print_str("Memory dump at ", WHITE);
    print_hex(address, CYAN);
    print_str(":", WHITE);
    print_new_line();
    
    uint8_t *ptr = (uint8_t*)address;
    
    for (uint32_t i = 0; i < size; i += 16) {
        print_hex(address + i, LIGHT_GREY);
        print_str(": ", LIGHT_GREY);
        
        /* Hex dump */
        for (uint32_t j = 0; j < 16 && (i + j) < size; j++) {
            if (j == 8) print_str(" ", WHITE);
            print_hex_short(ptr[i + j], WHITE);
            print_str(" ", WHITE);
        }
        
        /* ASCII dump */
        print_str(" |", LIGHT_GREY);
        for (uint32_t j = 0; j < 16 && (i + j) < size; j++) {
            char c = ptr[i + j];
            if (c >= 32 && c <= 126) {
                print_char(c, GREEN);
            } else {
                print_char('.', DARK_GREY);
            }
        }
        print_str("|", LIGHT_GREY);
        print_new_line();
    }
}

/* Test allocations for bonus */
void memory_test_allocations(void)
{
    print_str("=== Memory Allocation Test ===", CYAN);
    print_new_line();
    
    void *ptrs[5];
    size_t sizes[] = {64, 128, 256, 512, 1024};
    
    for (int i = 0; i < 5; i++) {
        ptrs[i] = kmalloc(sizes[i]);
        print_str("Allocated ", WHITE);
        print_dec(sizes[i], YELLOW);
        print_str(" bytes at ", WHITE);
        print_hex((uint32_t)ptrs[i], GREEN);
        print_new_line();
    }
    
    print_new_line();
    print_str("Freeing allocations...", WHITE);
    print_new_line();
    
    for (int i = 0; i < 5; i++) {
        kfree(ptrs[i]);
        print_str("Freed ", WHITE);
        print_dec(sizes[i], YELLOW);
        print_str(" bytes", WHITE);
        print_new_line();
    }
    
    print_str("Test complete!", GREEN);
    print_new_line();
}
