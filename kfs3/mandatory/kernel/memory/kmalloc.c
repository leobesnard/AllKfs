/*
 * Kernel Memory Allocator (kmalloc/kfree)
 * Simple heap allocator using first-fit algorithm
 */

#include "memory.h"
#include "screen.h"
#include "panic.h"

/* Heap block header */
typedef struct heap_block {
    uint32_t magic;             /* Magic number for validation */
    size_t size;                /* Size of data area (excluding header) */
    uint8_t free;               /* 1 if free, 0 if allocated */
    struct heap_block *next;    /* Next block in list */
    struct heap_block *prev;    /* Previous block in list */
} __attribute__((packed)) heap_block_t;

/* Magic values for validation */
#define HEAP_MAGIC          0xDEADBEEF
#define HEAP_MAGIC_FREE     0xFEEDFACE

/* Minimum block size (excluding header) */
#define MIN_BLOCK_SIZE      16

/* Heap state */
static heap_block_t *heap_start = NULL;
static heap_block_t *heap_end = NULL;
static uint32_t heap_base = 0;
static uint32_t heap_current = 0;
static uint32_t heap_limit = 0;
static size_t heap_total_allocated = 0;
static size_t heap_total_free = 0;

/* Align size to 4-byte boundary */
static inline size_t align_size(size_t size)
{
    return (size + 3) & ~3;
}

/* Initialize kernel heap */
void kheap_init(void)
{
    /* Use memory right after kernel for heap
     * We'll use physical memory directly (identity mapped) */
    uint32_t kernel_end_addr = (uint32_t)&kernel_end;
    
    /* Align to page boundary and skip the PMM bitmap area */
    uint32_t pmm_bitmap_size = (pmm_get_total_memory() / PAGE_SIZE + 31) / 32 * sizeof(uint32_t);
    heap_base = (kernel_end_addr + pmm_bitmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    /* Reserve 4MB for kernel heap */
    heap_limit = heap_base + (4 * 1024 * 1024);
    heap_current = heap_base;
    
    /* Create initial free block covering entire heap */
    heap_start = (heap_block_t*)heap_base;
    heap_start->magic = HEAP_MAGIC_FREE;
    heap_start->size = heap_limit - heap_base - sizeof(heap_block_t);
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_end = heap_start;
    heap_total_free = heap_start->size;
    heap_total_allocated = 0;
    
    print_str("Kernel Heap: ", WHITE);
    print_hex(heap_base, YELLOW);
    print_str(" - ", WHITE);
    print_hex(heap_limit, YELLOW);
    print_str(" (", LIGHT_GREY);
    print_dec((heap_limit - heap_base) / 1024, WHITE);
    print_str(" KB)", LIGHT_GREY);
    print_new_line();
}

/* Find a free block of at least 'size' bytes */
static heap_block_t* find_free_block(size_t size)
{
    heap_block_t *block = heap_start;
    
    while (block) {
        if (block->free && block->size >= size) {
            /* Validate magic */
            if (block->magic != HEAP_MAGIC_FREE) {
                kernel_panic("Heap corruption detected", __FILE__, __LINE__, PANIC_HEAP_CORRUPTION);
            }
            return block;
        }
        block = block->next;
    }
    
    return NULL;
}

/* Split a block if it's too large */
static void split_block(heap_block_t *block, size_t size)
{
    /* Only split if remaining space is large enough */
    if (block->size <= size + sizeof(heap_block_t) + MIN_BLOCK_SIZE) {
        return;
    }
    
    /* Create new block after allocated space */
    heap_block_t *new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
    new_block->magic = HEAP_MAGIC_FREE;
    new_block->size = block->size - size - sizeof(heap_block_t);
    new_block->free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    }
    
    block->next = new_block;
    block->size = size;
    
    if (heap_end == block) {
        heap_end = new_block;
    }
    
    heap_total_free += new_block->size;  /* Will be subtracted when block is marked used */
}

/* Allocate kernel memory */
void* kmalloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }
    
    /* Align size */
    size = align_size(size);
    
    /* Find free block */
    heap_block_t *block = find_free_block(size);
    
    if (block == NULL) {
        kernel_warning("kmalloc: Out of memory", __FILE__, __LINE__);
        return NULL;
    }
    
    /* Split block if necessary */
    split_block(block, size);
    
    /* Mark block as used */
    block->magic = HEAP_MAGIC;
    block->free = 0;
    
    heap_total_allocated += block->size;
    heap_total_free -= block->size;
    
    /* Return pointer to data area (after header) */
    return (void*)((uint8_t*)block + sizeof(heap_block_t));
}

/* Allocate aligned kernel memory */
void* kmalloc_aligned(size_t size, size_t alignment)
{
    if (size == 0 || alignment == 0) {
        return NULL;
    }
    
    /* Allocate extra space for alignment and storing offset */
    size_t total_size = size + alignment + sizeof(uint32_t);
    void *ptr = kmalloc(total_size);
    
    if (ptr == NULL) {
        return NULL;
    }
    
    /* Calculate aligned address */
    uint32_t addr = (uint32_t)ptr + sizeof(uint32_t);
    uint32_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
    
    /* Store offset before aligned address */
    *((uint32_t*)(aligned_addr - sizeof(uint32_t))) = aligned_addr - (uint32_t)ptr;
    
    return (void*)aligned_addr;
}

/* Merge adjacent free blocks */
static void merge_blocks(heap_block_t *block)
{
    /* Merge with next block if free */
    while (block->next && block->next->free) {
        if (block->next->magic != HEAP_MAGIC_FREE) {
            kernel_panic("Heap corruption during merge", __FILE__, __LINE__, PANIC_HEAP_CORRUPTION);
        }
        
        heap_total_free -= block->next->size;
        block->size += sizeof(heap_block_t) + block->next->size;
        
        if (heap_end == block->next) {
            heap_end = block;
        }
        
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    /* Merge with previous block if free */
    if (block->prev && block->prev->free) {
        if (block->prev->magic != HEAP_MAGIC_FREE) {
            kernel_panic("Heap corruption during merge", __FILE__, __LINE__, PANIC_HEAP_CORRUPTION);
        }
        
        heap_total_free -= block->prev->size;
        block->prev->size += sizeof(heap_block_t) + block->size;
        
        if (heap_end == block) {
            heap_end = block->prev;
        }
        
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        
        block = block->prev;
    }
    
    heap_total_free += block->size;
}

/* Free kernel memory */
void kfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    
    /* Get block header */
    heap_block_t *block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    
    /* Validate magic */
    if (block->magic != HEAP_MAGIC) {
        if (block->magic == HEAP_MAGIC_FREE) {
            kernel_panic("Double free detected", __FILE__, __LINE__, PANIC_DOUBLE_FREE);
        } else {
            kernel_panic("Invalid pointer or heap corruption", __FILE__, __LINE__, PANIC_INVALID_POINTER);
        }
        return;
    }
    
    /* Mark block as free */
    block->magic = HEAP_MAGIC_FREE;
    block->free = 1;
    
    heap_total_allocated -= block->size;
    
    /* Merge adjacent free blocks */
    merge_blocks(block);
}

/* Get size of allocated memory */
size_t ksize(void *ptr)
{
    if (ptr == NULL) {
        return 0;
    }
    
    /* Get block header */
    heap_block_t *block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    
    /* Validate magic */
    if (block->magic != HEAP_MAGIC) {
        return 0;
    }
    
    return block->size;
}

/* Extend/shrink kernel heap (sbrk-like) */
void* kbrk(intptr_t increment)
{
    if (increment == 0) {
        return (void*)heap_current;
    }
    
    uint32_t old_brk = heap_current;
    uint32_t new_brk = heap_current + increment;
    
    /* Check bounds */
    if (new_brk < heap_base || new_brk > heap_limit) {
        kernel_warning("kbrk: Out of bounds", __FILE__, __LINE__);
        return (void*)-1;
    }
    
    heap_current = new_brk;
    
    return (void*)old_brk;
}

/* Print heap information */
void kheap_print_info(void)
{
    print_str("=== Kernel Heap ===", CYAN);
    print_new_line();
    print_str("Base Address: ", WHITE);
    print_hex(heap_base, YELLOW);
    print_new_line();
    print_str("Limit:        ", WHITE);
    print_hex(heap_limit, YELLOW);
    print_new_line();
    print_str("Total Size:   ", WHITE);
    print_dec((heap_limit - heap_base) / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    print_str("Allocated:    ", WHITE);
    print_dec(heap_total_allocated / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    print_str("Free:         ", WHITE);
    print_dec(heap_total_free / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    print_new_line();
    
    /* List blocks */
    print_str("Heap Blocks:", GREEN);
    print_new_line();
    
    heap_block_t *block = heap_start;
    int count = 0;
    while (block && count < 10) {  /* Limit output */
        print_str("  [", LIGHT_GREY);
        print_hex((uint32_t)block, WHITE);
        print_str("] ", LIGHT_GREY);
        print_str(block->free ? "FREE " : "USED ", block->free ? GREEN : RED);
        print_dec(block->size, YELLOW);
        print_str(" bytes", WHITE);
        print_new_line();
        block = block->next;
        count++;
    }
    
    if (block) {
        print_str("  ... (more blocks)", LIGHT_GREY);
        print_new_line();
    }
}

/* Print overall memory information */
void memory_print_info(void)
{
    print_str("========================================", WHITE);
    print_new_line();
    print_str("       MEMORY INFORMATION", CYAN);
    print_new_line();
    print_str("========================================", WHITE);
    print_new_line();
    print_new_line();
    
    /* Physical memory */
    print_str("=== Physical Memory ===", CYAN);
    print_new_line();
    print_str("Total: ", WHITE);
    print_dec(pmm_get_total_memory() / 1024 / 1024, YELLOW);
    print_str(" MB", WHITE);
    print_new_line();
    print_str("Used:  ", WHITE);
    print_dec(pmm_get_used_memory() / 1024, YELLOW);
    print_str(" KB", WHITE);
    print_new_line();
    print_str("Free:  ", WHITE);
    print_dec(pmm_get_free_memory() / 1024 / 1024, YELLOW);
    print_str(" MB", WHITE);
    print_new_line();
    print_new_line();
    
    /* Kernel heap */
    kheap_print_info();
}
