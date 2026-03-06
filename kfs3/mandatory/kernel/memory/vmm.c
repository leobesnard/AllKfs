/*
 * Virtual Memory Manager (VMM)
 * Manages virtual address space allocation
 */

#include "memory.h"
#include "screen.h"
#include "panic.h"

/* Virtual memory region header */
typedef struct vmm_region {
    uint32_t start;             /* Start virtual address */
    uint32_t size;              /* Size in bytes (page-aligned) */
    struct vmm_region *next;    /* Next region in list */
    uint8_t flags;              /* Region flags */
    uint8_t allocated;          /* 1 if allocated, 0 if free */
} vmm_region_t;

/* VMM state */
static vmm_region_t *vmm_regions = NULL;
static uint32_t vmm_heap_start = 0;
static uint32_t vmm_heap_current = 0;
static uint32_t vmm_heap_end = 0;

/* Region pool for allocation tracking (statically allocated) */
#define VMM_MAX_REGIONS 256
static vmm_region_t region_pool[VMM_MAX_REGIONS];
static int region_pool_index = 0;

/* Allocate a region structure from pool */
static vmm_region_t* alloc_region(void)
{
    if (region_pool_index >= VMM_MAX_REGIONS) {
        kernel_warning("VMM: Region pool exhausted", __FILE__, __LINE__);
        return NULL;
    }
    
    vmm_region_t *region = &region_pool[region_pool_index++];
    memset(region, 0, sizeof(vmm_region_t));
    return region;
}

/* Initialize virtual memory manager */
void vmm_init(void)
{
    /* Set up virtual heap space */
    vmm_heap_start = KERNEL_HEAP_START;
    vmm_heap_current = vmm_heap_start;
    vmm_heap_end = KERNEL_HEAP_END;
    
    /* Initialize region pool */
    region_pool_index = 0;
    memset(region_pool, 0, sizeof(region_pool));
    
    /* Create initial free region covering entire heap */
    vmm_regions = alloc_region();
    if (vmm_regions) {
        vmm_regions->start = vmm_heap_start;
        vmm_regions->size = vmm_heap_end - vmm_heap_start;
        vmm_regions->next = NULL;
        vmm_regions->flags = 0;
        vmm_regions->allocated = 0;
    }
    
    print_str("VMM: Initialized virtual heap ", WHITE);
    print_hex(vmm_heap_start, YELLOW);
    print_str(" - ", WHITE);
    print_hex(vmm_heap_end, YELLOW);
    print_new_line();
}

/* Find a free region of at least 'size' bytes */
static vmm_region_t* find_free_region(size_t size)
{
    vmm_region_t *region = vmm_regions;
    
    while (region) {
        if (!region->allocated && region->size >= size) {
            return region;
        }
        region = region->next;
    }
    
    return NULL;
}

/* Allocate virtual memory */
void* vmalloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }
    
    /* Align size to page boundary */
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    /* Find a free region */
    vmm_region_t *region = find_free_region(size);
    if (region == NULL) {
        kernel_warning("VMM: Out of virtual memory", __FILE__, __LINE__);
        return NULL;
    }
    
    /* If region is larger than needed, split it */
    if (region->size > size) {
        vmm_region_t *new_region = alloc_region();
        if (new_region) {
            new_region->start = region->start + size;
            new_region->size = region->size - size;
            new_region->next = region->next;
            new_region->flags = 0;
            new_region->allocated = 0;
            
            region->size = size;
            region->next = new_region;
        }
    }
    
    /* Allocate physical pages and map them */
    uint32_t virtual_addr = region->start;
    for (uint32_t offset = 0; offset < size; offset += PAGE_SIZE) {
        void *physical = pmm_alloc_page();
        if (physical == NULL) {
            /* Rollback: unmap and free already allocated pages */
            for (uint32_t rollback = 0; rollback < offset; rollback += PAGE_SIZE) {
                uint32_t phys_addr = paging_get_physical(virtual_addr + rollback);
                if (phys_addr) {
                    pmm_free_page((void*)phys_addr);
                    paging_unmap_page(virtual_addr + rollback);
                }
            }
            region->allocated = 0;
            kernel_warning("VMM: Failed to allocate physical pages", __FILE__, __LINE__);
            return NULL;
        }
        
        paging_map_page(virtual_addr + offset, (uint32_t)physical, 
                       PTE_PRESENT | PTE_WRITABLE);
    }
    
    /* Mark region as allocated */
    region->allocated = 1;
    region->flags = MEM_KERNEL;
    
    return (void*)region->start;
}

/* Find region by address */
static vmm_region_t* find_region(void *ptr)
{
    uint32_t addr = (uint32_t)ptr;
    vmm_region_t *region = vmm_regions;
    
    while (region) {
        if (region->start == addr && region->allocated) {
            return region;
        }
        region = region->next;
    }
    
    return NULL;
}

/* Free virtual memory */
void vfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    
    vmm_region_t *region = find_region(ptr);
    if (region == NULL) {
        kernel_warning("VMM: Attempt to free invalid pointer", __FILE__, __LINE__);
        return;
    }
    
    /* Unmap and free physical pages */
    for (uint32_t offset = 0; offset < region->size; offset += PAGE_SIZE) {
        uint32_t phys_addr = paging_get_physical(region->start + offset);
        if (phys_addr) {
            pmm_free_page((void*)(phys_addr & 0xFFFFF000));
            paging_unmap_page(region->start + offset);
        }
    }
    
    /* Mark region as free */
    region->allocated = 0;
    
    /* Try to coalesce with adjacent free regions */
    vmm_region_t *current = vmm_regions;
    while (current && current->next) {
        if (!current->allocated && !current->next->allocated) {
            /* Merge current with next */
            current->size += current->next->size;
            current->next = current->next->next;
            /* Don't advance - check for more merges */
        } else {
            current = current->next;
        }
    }
}

/* Get size of allocated virtual memory */
size_t vsize(void *ptr)
{
    if (ptr == NULL) {
        return 0;
    }
    
    vmm_region_t *region = find_region(ptr);
    if (region == NULL) {
        return 0;
    }
    
    return region->size;
}

/* Extend/shrink virtual memory region (sbrk-like) */
void* vbrk(intptr_t increment)
{
    if (increment == 0) {
        return (void*)vmm_heap_current;
    }
    
    uint32_t old_brk = vmm_heap_current;
    uint32_t new_brk = vmm_heap_current + increment;
    
    /* Check bounds */
    if (new_brk < vmm_heap_start || new_brk > vmm_heap_end) {
        kernel_warning("VMM: vbrk out of bounds", __FILE__, __LINE__);
        return (void*)-1;
    }
    
    if (increment > 0) {
        /* Allocate pages for the new region */
        uint32_t start = (old_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint32_t end = (new_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        
        for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
            void *physical = pmm_alloc_page();
            if (physical == NULL) {
                kernel_warning("VMM: vbrk failed to allocate", __FILE__, __LINE__);
                return (void*)-1;
            }
            paging_map_page(addr, (uint32_t)physical, PTE_PRESENT | PTE_WRITABLE);
        }
    } else {
        /* Free pages for the shrunk region */
        uint32_t start = (new_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint32_t end = (old_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        
        for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
            uint32_t phys_addr = paging_get_physical(addr);
            if (phys_addr) {
                pmm_free_page((void*)(phys_addr & 0xFFFFF000));
                paging_unmap_page(addr);
            }
        }
    }
    
    vmm_heap_current = new_brk;
    return (void*)old_brk;
}

/* Print VMM information */
void vmm_print_info(void)
{
    print_str("=== Virtual Memory Manager ===", CYAN);
    print_new_line();
    print_str("Heap Start:   ", WHITE);
    print_hex(vmm_heap_start, YELLOW);
    print_new_line();
    print_str("Heap Current: ", WHITE);
    print_hex(vmm_heap_current, YELLOW);
    print_new_line();
    print_str("Heap End:     ", WHITE);
    print_hex(vmm_heap_end, YELLOW);
    print_new_line();
    print_new_line();
    
    /* Count regions */
    int total_regions = 0;
    int allocated_regions = 0;
    int free_regions = 0;
    size_t allocated_size = 0;
    size_t free_size = 0;
    
    vmm_region_t *region = vmm_regions;
    while (region) {
        total_regions++;
        if (region->allocated) {
            allocated_regions++;
            allocated_size += region->size;
        } else {
            free_regions++;
            free_size += region->size;
        }
        region = region->next;
    }
    
    print_str("Total Regions:     ", WHITE);
    print_dec(total_regions, YELLOW);
    print_new_line();
    print_str("Allocated Regions: ", WHITE);
    print_dec(allocated_regions, YELLOW);
    print_str(" (", LIGHT_GREY);
    print_dec(allocated_size / 1024, WHITE);
    print_str(" KB)", LIGHT_GREY);
    print_new_line();
    print_str("Free Regions:      ", WHITE);
    print_dec(free_regions, YELLOW);
    print_str(" (", LIGHT_GREY);
    print_dec(free_size / 1024 / 1024, WHITE);
    print_str(" MB)", LIGHT_GREY);
    print_new_line();
}
