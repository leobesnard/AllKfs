/**
 * =============================================================================
 * gdt.h - Global Descriptor Table Management Header
 * =============================================================================
 * 
 * This header defines the interface for GDT (Global Descriptor Table)
 * management in protected mode x86 systems. The GDT contains segment
 * descriptors that define memory segments for the CPU.
 * 
 * Segment Layout:
 *   - Entry 0: Null descriptor (required by CPU)
 *   - Entry 1: Kernel code segment (Ring 0)
 *   - Entry 2: Kernel data segment (Ring 0)
 *   - Entry 3: Kernel stack segment (Ring 0)
 *   - Entry 4: User code segment (Ring 3)
 *   - Entry 5: User data segment (Ring 3)
 *   - Entry 6: User stack segment (Ring 3)
 * 
 * =============================================================================
 */

#ifndef GDT_H
# define GDT_H

/* =============================================================================
 * GDT Memory Configuration
 * ============================================================================= */

# define GDT_BASE_ADDRESS   0x800

/* =============================================================================
 * External References
 * ============================================================================= */

extern int kernel_stack_top;

/* =============================================================================
 * Function Prototypes - Display Functions
 * ============================================================================= */

/**
 * Display the GDT entries with their addresses and access rights.
 */
void gdt_display(void);

/**
 * Display kernel stack information including base, current pointer, and content.
 */
void stack_display(void);

/**
 * Print an unsigned integer value in hexadecimal format (0xXXXXXXXX).
 * 
 * @param value The value to display in hexadecimal
 * @param color The color attribute for the output
 */
void hex_print(unsigned int value, unsigned char color);

/* =============================================================================
 * Function Prototypes - GDT Management
 * ============================================================================= */

/**
 * Create a GDT segment descriptor entry.
 * 
 * @param index  The index in the GDT table (0-6)
 * @param base   The base address of the segment
 * @param limit  The segment limit (max offset)
 * @param access The access byte (P, DPL, S, Type flags)
 * @param gran   The granularity byte (G, D/B, L, AVL flags)
 */
void gdt_create_entry(int index, unsigned int base, unsigned int limit,
                      unsigned char access, unsigned char gran);

/**
 * Initialize the Global Descriptor Table with all required segments.
 * Sets up null, kernel, and user segments then loads the GDT.
 */
void gdt_init(void);

#endif /* GDT_H */
