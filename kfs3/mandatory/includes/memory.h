#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "multiboot.h"

/* Page size constants */
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGES_PER_TABLE     1024
#define TABLES_PER_DIR      1024

/* Memory regions */
#define KERNEL_VIRTUAL_BASE 0xC0000000      // 3GB
#define KERNEL_HEAP_START   0xD0000000      // Start of kernel heap
#define KERNEL_HEAP_END     0xE0000000      // End of kernel heap (256MB)
#define USER_SPACE_END      0xBFFFFFFF      // End of user space
#define USER_HEAP_START     0x10000000      // Start of user heap

/* Page table entry flags */
#define PTE_PRESENT         0x001           // P - Page is present
#define PTE_WRITABLE        0x002           // R/W - Page is writable
#define PTE_USER            0x004           // U/S - User accessible
#define PTE_WRITETHROUGH    0x008           // PWT - Write-through caching
#define PTE_NOCACHE         0x010           // PCD - Cache disabled
#define PTE_ACCESSED        0x020           // A - Has been accessed
#define PTE_DIRTY           0x040           // D - Has been written to
#define PTE_LARGE           0x080           // PS - 4MB page (only for PDE)
#define PTE_GLOBAL          0x100           // G - Global page

/* Memory flags for allocation */
#define MEM_KERNEL          0x00            // Kernel space allocation
#define MEM_USER            0x01            // User space allocation

/* Forward declarations */
typedef uint32_t page_t;
typedef uint32_t* page_table_t;
typedef uint32_t* page_directory_t;

/* External symbols from linker */
extern uint32_t kernel_start;
extern uint32_t kernel_end;

/* ================= Physical Memory Manager (PMM) ================= */

/* Initialize physical memory manager */
void pmm_init(multiboot_info_t *mboot);

/* Allocate a physical page */
void* pmm_alloc_page(void);

/* Free a physical page */
void pmm_free_page(void *page);

/* Get total memory size */
uint32_t pmm_get_total_memory(void);

/* Get free memory size */
uint32_t pmm_get_free_memory(void);

/* Get used memory size */
uint32_t pmm_get_used_memory(void);

/* ================= Paging ================= */

/* Initialize paging */
void paging_init(void);

/* Enable paging */
void paging_enable(void);

/* Map a virtual address to a physical address */
void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);

/* Unmap a virtual address */
void paging_unmap_page(uint32_t virtual_addr);

/* Get physical address from virtual address */
uint32_t paging_get_physical(uint32_t virtual_addr);

/* Allocate a new page table */
page_table_t paging_alloc_table(void);

/* ================= Virtual Memory Manager (VMM) ================= */

/* Initialize virtual memory manager */
void vmm_init(void);

/* Allocate virtual memory pages */
void* vmalloc(size_t size);

/* Free virtual memory */
void vfree(void *ptr);

/* Get size of allocated virtual memory */
size_t vsize(void *ptr);

/* Extend/shrink virtual memory region (like sbrk) */
void* vbrk(intptr_t increment);

/* ================= Kernel Memory Allocator ================= */

/* Initialize kernel heap */
void kheap_init(void);

/* Allocate kernel memory (physical) */
void* kmalloc(size_t size);

/* Allocate aligned kernel memory */
void* kmalloc_aligned(size_t size, size_t alignment);

/* Free kernel memory */
void kfree(void *ptr);

/* Get size of allocated kernel memory */
size_t ksize(void *ptr);

/* Extend/shrink kernel heap (like sbrk) */
void* kbrk(intptr_t increment);

/* ================= Memory Utilities ================= */

/* Memory copy */
void* memcpy(void *dest, const void *src, size_t n);

/* Memory set */
void* memset(void *s, int c, size_t n);

/* Memory compare */
int memcmp(const void *s1, const void *s2, size_t n);

/* Print memory info */
void memory_print_info(void);

#endif
