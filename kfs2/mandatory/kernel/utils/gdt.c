/**
 * =============================================================================
 * gdt.c - Global Descriptor Table Implementation
 * =============================================================================
 * 
 * This file implements the GDT (Global Descriptor Table) management for
 * protected mode x86. The GDT defines memory segments with different
 * privilege levels for kernel and user space code execution.
 * 
 * Segment Descriptor Structure (8 bytes per entry):
 *   - Limit (20 bits): Maximum addressable unit
 *   - Base (32 bits): Starting address of segment
 *   - Access byte: Permissions and type flags
 *   - Flags: Granularity and size bits
 * 
 * =============================================================================
 */

#include "gdt.h"
#include "screen.h"

/* =============================================================================
 * Type Definitions
 * ============================================================================= */

/**
 * GDT Entry Structure
 * 
 * Packed structure representing a single GDT segment descriptor.
 * The fields are split across the structure for historical x86 reasons.
 */
struct gdt_entry_t
{
    unsigned short limit_low;       /* Limit bits 0-15 */
    unsigned short base_low;        /* Base address bits 0-15 */
    unsigned char  base_middle;     /* Base address bits 16-23 */
    unsigned char  access;          /* Access flags byte */
    unsigned char  granularity;     /* Granularity and limit bits 16-19 */
    unsigned char  base_high;       /* Base address bits 24-31 */
} __attribute__((packed));

/**
 * GDTR Value Structure
 * 
 * Structure loaded into the GDTR register via LGDT instruction.
 * Contains the limit (size - 1) and linear base address of GDT.
 */
struct
{
    unsigned short limit;
    unsigned int base;
} gdtr_value;

/* =============================================================================
 * Global Variables
 * ============================================================================= */

/* Pointer to the GDT table located at fixed memory address */
struct gdt_entry_t *gdt_table = (struct gdt_entry_t *)GDT_BASE_ADDRESS;

/* =============================================================================
 * Utility Functions
 * ============================================================================= */

/**
 * hex_print - Display an unsigned integer in hexadecimal format
 * 
 * Converts a 32-bit value to its hexadecimal string representation
 * with '0x' prefix and displays it using the VGA driver.
 * 
 * @param value The 32-bit value to convert and display
 * @param color The color attribute for the output
 */
void hex_print(unsigned int value, unsigned char color)
{
    const char hex_digits[] = "0123456789ABCDEF";
    char hex_str[11];
    int i;

    hex_str[0] = '0';
    hex_str[1] = 'x';

    for (i = 0; i < 8; i++)
    {
        /* Extract each nibble from most significant to least significant */
        hex_str[2 + i] = hex_digits[(value >> (28 - i * 4)) & 0xF];
    }

    hex_str[10] = '\0';
    vga_puts(hex_str, color);
}

/**
 * gdt_get_base - Reconstruct full base address from GDT entry
 * 
 * Combines the three separate base address fields from a GDT entry
 * into a single 32-bit linear address.
 * 
 * @param entry Pointer to the GDT entry to extract base from
 * @return      The complete 32-bit base address
 */
static unsigned int gdt_get_base(struct gdt_entry_t *entry)
{
    unsigned int base;

    base = 0;
    base |= (entry->base_low);              /* Bits 0-15 */
    base |= (entry->base_middle << 16);     /* Bits 16-23 */
    base |= (entry->base_high << 24);       /* Bits 24-31 */
    return base;
}

/* =============================================================================
 * GDT Entry Creation
 * ============================================================================= */

/**
 * gdt_create_entry - Initialize a GDT segment descriptor
 * 
 * Populates a GDT entry at the specified index with the given
 * segment parameters. Handles the split field layout of x86 descriptors.
 * 
 * @param index  Index in the GDT table (0-6)
 * @param base   32-bit base address of the segment
 * @param limit  20-bit segment limit
 * @param access Access byte with permission flags
 * @param gran   Granularity byte with size flags
 */
void gdt_create_entry(int index, unsigned int base, unsigned int limit,
                      unsigned char access, unsigned char gran)
{
    /* Set the base address (split across three fields) */
    gdt_table[index].base_low = (base & 0xFFFF);
    gdt_table[index].base_middle = (base >> 16) & 0xFF;
    gdt_table[index].base_high = (base >> 24) & 0xFF;

    /* Set the limit (split across two fields) */
    gdt_table[index].limit_low = (limit & 0xFFFF);
    gdt_table[index].granularity = ((limit >> 16) & 0x0F);

    /* Set granularity flags and access byte */
    gdt_table[index].granularity |= gran & 0xF0;
    gdt_table[index].access = access;
}

/* =============================================================================
 * GDT Initialization
 * ============================================================================= */

/**
 * gdt_init - Initialize the Global Descriptor Table
 * 
 * Creates all required segment descriptors for the kernel:
 *   - Null descriptor (required by CPU, index 0)
 *   - Kernel code segment (index 1, ring 0)
 *   - Kernel data segment (index 2, ring 0)
 *   - Kernel stack segment (index 3, ring 0)
 *   - User code segment (index 4, ring 3)
 *   - User data segment (index 5, ring 3)
 *   - User stack segment (index 6, ring 3)
 * 
 * After creating entries, retrieves GDT info via SGDT instruction
 * and displays the GDT base address and size.
 */
void gdt_init(void)
{
    /* Null descriptor - required by x86 architecture */
    gdt_create_entry(0, 0, 0, 0, 0);

    /* Kernel Code Segment (Offset: 0x08)
     * Base: 0, Limit: 0xFFFFF, Access: 0x9A (Present, Ring 0, Code, Executable, Readable) */
    gdt_create_entry(1, 0, 0xFFFFF, 0x9A, 0xC0);

    /* Kernel Data Segment (Offset: 0x10)
     * Base: 0, Limit: 0xFFFFF, Access: 0x92 (Present, Ring 0, Data, Writable) */
    gdt_create_entry(2, 0, 0xFFFFF, 0x92, 0xC0);

    /* Kernel Stack Segment (Offset: 0x18)
     * Base: 0, Limit: 0xFFFFF, Access: 0x92 (Present, Ring 0, Data, Writable) */
    gdt_create_entry(3, 0, 0xFFFFF, 0x92, 0xC0);

    /* User Code Segment (Offset: 0x20)
     * Base: 0, Limit: 0xFFFFF, Access: 0xFA (Present, Ring 3, Code, Executable, Readable) */
    gdt_create_entry(4, 0, 0xFFFFF, 0xFA, 0xC0);

    /* User Data Segment (Offset: 0x28)
     * Base: 0, Limit: 0xFFFFF, Access: 0xF2 (Present, Ring 3, Data, Writable) */
    gdt_create_entry(5, 0, 0xFFFFF, 0xF2, 0xC0);

    /* User Stack Segment (Offset: 0x30)
     * Base: 0, Limit: 0xFFFFF, Access: 0xF2 (Present, Ring 3, Data, Writable) */
    gdt_create_entry(6, 0, 0xFFFFF, 0xF2, 0xC0);

    /* Retrieve the current GDTR value using SGDT instruction */
    asm volatile("sgdt %0" : "=m" (gdtr_value));
    gdtr_value.base = (unsigned int)gdt_table;

    /* Display GDT configuration information */
    vga_puts("---- GDT Descriptors---- ", RED);
    vga_newline();
    vga_puts("GDT Base Address: ", WHITE);
    hex_print(gdtr_value.base, MAGENTA);
    vga_newline();
    vga_puts("GDT Size: ", WHITE);
    hex_print(gdtr_value.limit, MAGENTA);
    vga_newline();
    vga_newline();
}

/* =============================================================================
 * Display Functions
 * ============================================================================= */

/**
 * gdt_display - Show all GDT entries with their properties
 * 
 * Iterates through all 7 GDT entries and displays:
 *   - Segment name (Null, Kernel Code, etc.)
 *   - Memory address of the entry
 *   - Access byte value
 */
void gdt_display(void)
{
    char *segments[9] = {
        "Null",
        "Kernel Code",
        "Kernel Data",
        "Kernel Stack",
        "User Code",
        "User Data",
        "User Stack"
    };
    struct gdt_entry_t *entry;
    int i;

    vga_puts("---- GDT Registres: ----", RED);
    vga_newline();

    for (i = 0; i < 7; i++)
    {
        entry = gdt_table + i;
        vga_puts(segments[i], WHITE);
        vga_puts(" adress ", GREEN);
        hex_print((unsigned int)&gdt_table[i], GREEN);
        vga_puts(" | Access: ", YELLOW);
        hex_print(entry->access, YELLOW);
        vga_newline();
    }
}

/**
 * stack_display - Display kernel stack information
 * 
 * Shows the current state of the kernel stack including:
 *   - Current stack pointer address (ESP)
 *   - Stack base address (top of reserved stack space)
 *   - Current stack size in bytes
 *   - Content of the first 4 stack entries
 */
void stack_display(void)
{
    int *stack_ptr;
    int stack_base;
    int value;
    int i;

    stack_base = (int)&kernel_stack_top;

    /* Retrieve current stack pointer via inline assembly */
    asm volatile("movl %%esp, %0" : "=r"(stack_ptr));

    /* Display stack location information */
    vga_newline();
    vga_puts("---- KERNEL STACK ---- ", RED);
    vga_newline();
    vga_puts("Stack Address: ", WHITE);
    hex_print((unsigned int)stack_ptr, LIGHT_CYAN);
    vga_newline();
    vga_puts("Stack Base: ", WHITE);
    hex_print(stack_base, LIGHT_CYAN);
    vga_newline();
    vga_puts("Stack Size: ", WHITE);
    hex_print(stack_base - (unsigned int)stack_ptr, LIGHT_CYAN);
    vga_puts(" bytes", LIGHT_CYAN);
    vga_newline();

    /* Display stack content (top 4 values) */
    vga_newline();
    vga_puts("---- STACK CONTENT ---- ", RED);
    vga_newline();

    for (i = 0; i < 4 && stack_ptr < (int *)stack_base; i++)
    {
        vga_puts("Stack[", LIGHT_GREEN);
        hex_print((unsigned int)stack_ptr, LIGHT_GREEN);
        vga_puts("]: ", LIGHT_GREEN);
        value = *stack_ptr;
        hex_print(value, WHITE);
        stack_ptr++;
        vga_newline();
    }
}
