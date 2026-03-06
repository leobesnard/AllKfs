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
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_HEAP_START   0xD0000000
#define KERNEL_HEAP_END     0xE0000000
#define USER_SPACE_END      0xBFFFFFFF
#define USER_HEAP_START     0x10000000

/* Page table entry flags */
#define PTE_PRESENT         0x001
#define PTE_WRITABLE        0x002
#define PTE_USER            0x004
#define PTE_WRITETHROUGH    0x008
#define PTE_NOCACHE         0x010
#define PTE_ACCESSED        0x020
#define PTE_DIRTY           0x040
#define PTE_LARGE           0x080
#define PTE_GLOBAL          0x100

/* Memory flags for allocation */
#define MEM_KERNEL          0x00
#define MEM_USER            0x01

/* Forward declarations */
typedef uint32_t page_t;
typedef uint32_t* page_table_t;
typedef uint32_t* page_directory_t;

/* External symbols from linker */
extern uint32_t kernel_start;
extern uint32_t kernel_end;

/* ================= Physical Memory Manager (PMM) ================= */
void pmm_init(multiboot_info_t *mboot);
void* pmm_alloc_page(void);
void pmm_free_page(void *page);
uint32_t pmm_get_total_memory(void);
uint32_t pmm_get_free_memory(void);
uint32_t pmm_get_used_memory(void);
uint32_t pmm_get_total_pages(void);
uint32_t pmm_get_used_pages(void);

/* ================= Paging ================= */
void paging_init(void);
void paging_enable(void);
void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void paging_unmap_page(uint32_t virtual_addr);
uint32_t paging_get_physical(uint32_t virtual_addr);
page_table_t paging_alloc_table(void);
void paging_print_info(void);

/* ================= Virtual Memory Manager (VMM) ================= */
void vmm_init(void);
void* vmalloc(size_t size);
void vfree(void *ptr);
size_t vsize(void *ptr);
void* vbrk(intptr_t increment);
void vmm_print_info(void);

/* ================= Kernel Memory Allocator ================= */
void kheap_init(void);
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t alignment);
void kfree(void *ptr);
size_t ksize(void *ptr);
void* kbrk(intptr_t increment);
void kheap_print_info(void);

/* ================= Memory Utilities ================= */
void* memcpy(void *dest, const void *src, size_t n);
void* memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void memory_print_info(void);

/* ================= Memory Debug (Bonus) ================= */
void memory_dump(uint32_t address, uint32_t size);
void memory_dump_page(uint32_t page_num);
void memory_test_allocations(void);

#endif
