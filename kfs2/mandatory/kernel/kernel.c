/**
 * =============================================================================
 * kernel.c - Kernel Entry Point
 * =============================================================================
 * 
 * Main kernel initialization and execution flow. This is the C entry point
 * called from the assembly boot code after basic system initialization.
 * 
 * Responsibilities:
 *   - Initialize the VGA display driver
 *   - Configure the Global Descriptor Table (GDT)
 *   - Display system information (GDT entries, kernel stack)
 * 
 * =============================================================================
 */

#include "screen.h"
#include "gdt.h"

/* =============================================================================
 * Kernel Entry Point
 * ============================================================================= */

/**
 * main - Primary kernel function
 * 
 * Called by the bootloader after the CPU is in protected mode.
 * Performs initial system setup and displays diagnostic information.
 */
void main(void)
{
    /* Initialize VGA display driver */
    vga_buffer = (unsigned short *)VGA_MEMORY_BASE;
    vga_clear();

    /* Configure and display GDT information */
    gdt_init();
    gdt_display();

    /* Display kernel stack state */
    stack_display();
    vga_newline();
}
