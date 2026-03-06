/*
 * KFS-3 Kernel Main
 * Memory Management Implementation
 */

#include "types.h"
#include "screen.h"
#include "gdt.h"
#include "memory.h"
#include "multiboot.h"
#include "panic.h"

/* External multiboot values from boot.asm */
extern uint32_t multiboot_magic;
extern uint32_t multiboot_info;

/* Test memory allocations */
static void test_memory(void)
{
    print_str("=== Testing Memory Allocation ===", CYAN);
    print_new_line();
    print_new_line();
    
    /* Test kmalloc */
    print_str("1. Testing kmalloc/kfree:", GREEN);
    print_new_line();
    
    void *ptr1 = kmalloc(100);
    print_str("   Allocated 100 bytes at: ", WHITE);
    print_hex((uint32_t)ptr1, YELLOW);
    print_new_line();
    
    void *ptr2 = kmalloc(256);
    print_str("   Allocated 256 bytes at: ", WHITE);
    print_hex((uint32_t)ptr2, YELLOW);
    print_new_line();
    
    void *ptr3 = kmalloc(1024);
    print_str("   Allocated 1024 bytes at: ", WHITE);
    print_hex((uint32_t)ptr3, YELLOW);
    print_new_line();
    
    /* Get sizes */
    print_str("   Size of ptr1: ", WHITE);
    print_dec(ksize(ptr1), YELLOW);
    print_str(" bytes", WHITE);
    print_new_line();
    
    /* Write some data */
    char *test_str = (char*)ptr1;
    test_str[0] = 'H';
    test_str[1] = 'e';
    test_str[2] = 'l';
    test_str[3] = 'l';
    test_str[4] = 'o';
    test_str[5] = '\0';
    print_str("   Written to ptr1: ", WHITE);
    print_str(test_str, YELLOW);
    print_new_line();
    
    /* Free memory */
    print_str("   Freeing ptr2...", WHITE);
    kfree(ptr2);
    print_str(" Done", GREEN);
    print_new_line();
    
    /* Reallocate */
    void *ptr4 = kmalloc(128);
    print_str("   Reallocated 128 bytes at: ", WHITE);
    print_hex((uint32_t)ptr4, YELLOW);
    print_new_line();
    
    /* Free all */
    kfree(ptr1);
    kfree(ptr3);
    kfree(ptr4);
    print_str("   All memory freed", GREEN);
    print_new_line();
    print_new_line();
    
    /* Test vmalloc */
    print_str("2. Testing vmalloc/vfree:", GREEN);
    print_new_line();
    
    void *vptr1 = vmalloc(PAGE_SIZE);
    print_str("   Allocated 4KB virtual at: ", WHITE);
    print_hex((uint32_t)vptr1, YELLOW);
    print_new_line();
    
    void *vptr2 = vmalloc(PAGE_SIZE * 2);
    print_str("   Allocated 8KB virtual at: ", WHITE);
    print_hex((uint32_t)vptr2, YELLOW);
    print_new_line();
    
    /* Get virtual size */
    print_str("   Size of vptr1: ", WHITE);
    print_dec(vsize(vptr1), YELLOW);
    print_str(" bytes", WHITE);
    print_new_line();
    
    /* Free virtual memory */
    vfree(vptr1);
    vfree(vptr2);
    print_str("   Virtual memory freed", GREEN);
    print_new_line();
    print_new_line();
    
    /* Test physical page allocation */
    print_str("3. Testing PMM:", GREEN);
    print_new_line();
    
    void *page1 = pmm_alloc_page();
    print_str("   Allocated page at: ", WHITE);
    print_hex((uint32_t)page1, YELLOW);
    print_new_line();
    
    void *page2 = pmm_alloc_page();
    print_str("   Allocated page at: ", WHITE);
    print_hex((uint32_t)page2, YELLOW);
    print_new_line();
    
    pmm_free_page(page1);
    pmm_free_page(page2);
    print_str("   Physical pages freed", GREEN);
    print_new_line();
    print_new_line();
}

/* Main kernel entry point */
void main(uint32_t magic, multiboot_info_t *mboot)
{
    /* Initialize screen */
    screen_buffer = (uint16_t*)VGA_ADDRESS;
    clear_screen();
    
    /* Print header */
    print_str("========================================", CYAN);
    print_new_line();
    print_str("       KFS-3: Memory Management", WHITE);
    print_new_line();
    print_str("========================================", CYAN);
    print_new_line();
    print_new_line();
    
    /* Validate multiboot */
    if (magic != MULTIBOOT_MAGIC) {
        print_str("ERROR: Invalid multiboot magic: ", RED);
        print_hex(magic, WHITE);
        print_new_line();
        print_str("Expected: ", YELLOW);
        print_hex(MULTIBOOT_MAGIC, WHITE);
        print_new_line();
        PANIC("Invalid multiboot magic number");
    }
    
    print_str("Multiboot magic: ", WHITE);
    print_hex(magic, GREEN);
    print_str(" OK", GREEN);
    print_new_line();
    print_str("Multiboot info:  ", WHITE);
    print_hex((uint32_t)mboot, GREEN);
    print_new_line();
    print_new_line();
    
    /* Display memory info from multiboot */
    if (mboot->flags & MULTIBOOT_FLAG_MEM) {
        print_str("Memory (from BIOS):", YELLOW);
        print_new_line();
        print_str("  Lower: ", WHITE);
        print_dec(mboot->mem_lower, CYAN);
        print_str(" KB", WHITE);
        print_new_line();
        print_str("  Upper: ", WHITE);
        print_dec(mboot->mem_upper, CYAN);
        print_str(" KB (", WHITE);
        print_dec(mboot->mem_upper / 1024, CYAN);
        print_str(" MB)", WHITE);
        print_new_line();
        print_new_line();
    }
    
    /* Initialize Physical Memory Manager */
    print_str("Initializing Physical Memory Manager...", WHITE);
    print_new_line();
    pmm_init(mboot);
    print_str("PMM: Total ", WHITE);
    print_dec(pmm_get_total_memory() / 1024 / 1024, GREEN);
    print_str(" MB, Free ", WHITE);
    print_dec(pmm_get_free_memory() / 1024 / 1024, GREEN);
    print_str(" MB", WHITE);
    print_new_line();
    print_new_line();
    
    /* Initialize Paging */
    print_str("Initializing Paging...", WHITE);
    print_new_line();
    paging_init();
    paging_enable();
    print_new_line();
    
    /* Initialize Virtual Memory Manager */
    print_str("Initializing Virtual Memory Manager...", WHITE);
    print_new_line();
    vmm_init();
    print_new_line();
    
    /* Initialize Kernel Heap */
    print_str("Initializing Kernel Heap...", WHITE);
    print_new_line();
    kheap_init();
    print_new_line();
    
    /* Print GDT info */
    print_gdt();
    print_new_line();
    
    /* Print kernel stack info */
    print_kernel_stack();
    print_new_line();
    
    /* Test memory allocation */
    test_memory();
    
    /* Print final memory status */
    memory_print_info();
    
    print_new_line();
    print_str("========================================", CYAN);
    print_new_line();
    print_str("   KFS-3 Initialization Complete!", GREEN);
    print_new_line();
    print_str("========================================", CYAN);
    print_new_line();
    
    /* Halt */
    print_new_line();
    print_str("System halted. Press Ctrl+Alt+Del to reboot.", LIGHT_GREY);
    
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
