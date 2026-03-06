/*
 * KFS-3 Kernel Main - Bonus Version
 * Memory Management with Shell
 */

#include "types.h"
#include "screen.h"
#include "gdt.h"
#include "memory.h"
#include "multiboot.h"
#include "panic.h"
#include "keyboard.h"

extern uint32_t multiboot_magic;
extern uint32_t multiboot_info;

/* Initialize all screens */
static void init_screens(void)
{
    screen_buffer = (uint16_t*)VGA_ADDRESS;
    
    for (int i = 0; i < SCREEN_COUNT; i++) {
        switch_screen(i);
        clear_screen();
        print_str("KFS-3 Screen ", GREEN);
        print_dec(i + 1, YELLOW);
        print_str(" - Memory Management", GREEN);
        print_new_line();
        print_str("Type 'help' for commands", LIGHT_GREY);
        print_new_line();
    }
    
    switch_screen(0);
}

void main(uint32_t magic, multiboot_info_t *mboot)
{
    /* Initialize screen */
    screen_buffer = (uint16_t*)VGA_ADDRESS;
    clear_screen();
    
    /* Print header */
    print_str("========================================", CYAN);
    print_new_line();
    print_str("   KFS-3 Bonus: Memory Management", WHITE);
    print_new_line();
    print_str("========================================", CYAN);
    print_new_line();
    print_new_line();
    
    /* Validate multiboot */
    if (magic != MULTIBOOT_MAGIC) {
        print_str("ERROR: Invalid multiboot magic!", RED);
        print_new_line();
        PANIC("Invalid multiboot magic");
    }
    
    print_str("Multiboot: ", WHITE);
    print_hex(magic, GREEN);
    print_str(" OK", GREEN);
    print_new_line();
    
    /* Show memory from BIOS */
    if (mboot->flags & MULTIBOOT_FLAG_MEM) {
        print_str("Memory: ", WHITE);
        print_dec((mboot->mem_upper + 1024) / 1024, CYAN);
        print_str(" MB", WHITE);
        print_new_line();
    }
    print_new_line();
    
    /* Initialize PMM */
    print_str("Initializing PMM...", WHITE);
    pmm_init(mboot);
    print_str(" OK", GREEN);
    print_new_line();
    
    /* Initialize Paging */
    print_str("Initializing Paging...", WHITE);
    paging_init();
    paging_enable();
    print_str(" OK", GREEN);
    print_new_line();
    
    /* Initialize VMM */
    print_str("Initializing VMM...", WHITE);
    vmm_init();
    print_str(" OK", GREEN);
    print_new_line();
    
    /* Initialize Kernel Heap */
    print_str("Initializing Heap...", WHITE);
    kheap_init();
    print_str(" OK", GREEN);
    print_new_line();
    print_new_line();
    
    /* Print summary */
    print_str("=== Memory Summary ===", CYAN);
    print_new_line();
    print_str("Physical: ", WHITE);
    print_dec(pmm_get_total_memory() / 1024 / 1024, YELLOW);
    print_str(" MB total, ", WHITE);
    print_dec(pmm_get_free_memory() / 1024 / 1024, GREEN);
    print_str(" MB free", WHITE);
    print_new_line();
    print_str("Paging:   ", WHITE);
    print_str("ENABLED", GREEN);
    print_new_line();
    print_new_line();
    
    print_str("========================================", CYAN);
    print_new_line();
    print_str("   Initialization Complete!", GREEN);
    print_new_line();
    print_str("========================================", CYAN);
    print_new_line();
    print_new_line();
    
    /* Initialize multiple screens */
    init_screens();
    
    /* Show prompt on screen 0 */
    switch_screen(0);
    print_new_line();
    print_str("Controls: Ctrl+1/2/3 switch screens", LIGHT_GREY);
    print_new_line();
    print_str("          Arrow Up/Down scroll", LIGHT_GREY);
    print_new_line();
    new_prompt();
    
    /* Handle keyboard input */
    handle_keyboard();
}
