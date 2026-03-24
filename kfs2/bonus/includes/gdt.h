/*
 * =============================================================================
 *                              KFS2 BONUS - GDT HEADER
 * =============================================================================
 * Global Descriptor Table management header file
 * Provides GDT initialization and display functions
 * =============================================================================
 */

#ifndef GDT_H
# define GDT_H

/* Base address where GDT is loaded in memory */
# define GDT_BASE_ADDRESS   0x800

/* External reference to kernel stack top (defined in boot.asm) */
extern int kernel_stack_top;

/* =============================================================================
 *                              GDT DISPLAY FUNCTIONS
 * ============================================================================= */

/*
 * gdt_display - Display GDT entries information
 * Shows segment names, addresses, and access bytes
 */
void gdt_display(void);

/*
 * stack_display - Display kernel stack information
 * Shows stack pointer, base address, and top stack values
 */
void stack_display(void);

/*
 * hex_print - Print an unsigned integer in hexadecimal format
 * @value: The value to print
 * @color: VGA color attribute
 */
void hex_print(unsigned int value, unsigned char color);

/* =============================================================================
 *                              GDT SETUP FUNCTIONS
 * ============================================================================= */

/*
 * gdt_create_entry - Create a GDT descriptor entry
 * @index: GDT entry index (0-6)
 * @base: Segment base address
 * @limit: Segment limit
 * @access: Access byte (permissions and type)
 * @gran: Granularity byte (flags and limit high bits)
 */
void gdt_create_entry(int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran);

/*
 * gdt_init - Initialize the Global Descriptor Table
 * Sets up NULL, kernel, and user segment descriptors
 */
void gdt_init(void);

#endif
