/*
 * =============================================================================
 *                              KFS2 BONUS - GDT DRIVER
 * =============================================================================
 * Global Descriptor Table initialization and display functions
 * Manages memory segmentation for kernel and user mode
 * =============================================================================
 */

#include "gdt.h"
#include "screen.h"
#include "ft_printf.h"

/* =============================================================================
 *                              GDT ENTRY STRUCTURE
 * ============================================================================= */

/*
 * struct gdt_entry_t - GDT descriptor entry (8 bytes)
 *
 * The x86 GDT entry format splits the base and limit across multiple fields
 * for backward compatibility with older processors.
 *
 * @limit_low:   Segment limit bits 0-15
 * @base_low:    Segment base address bits 0-15
 * @base_middle: Segment base address bits 16-23
 * @access:      Access byte (type, DPL, present bit)
 * @granularity: Flags and limit bits 16-19
 * @base_high:   Segment base address bits 24-31
 */
struct gdt_entry_t
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

/*
 * GDT descriptor pointer structure
 * Used with LGDT/SGDT instructions
 */
struct
{
    unsigned short limit;
    unsigned int base;
} gdt_ptr;

/* Pointer to GDT in memory at GDT_BASE_ADDRESS */
struct gdt_entry_t *gdt = (struct gdt_entry_t*)GDT_BASE_ADDRESS;

/* =============================================================================
 *                              UTILITY FUNCTIONS
 * ============================================================================= */

/*
 * hex_print - Print an unsigned integer in hexadecimal format
 * @value: Value to print
 * @color: VGA color attribute
 *
 * Outputs in format: 0xXXXXXXXX (8 hex digits)
 */
void hex_print(unsigned int value, unsigned char color)
{
    const char hex_digits[] = "0123456789ABCDEF";
    char hex_str[11];   /* "0x" + 8 digits + null terminator */

    hex_str[0] = '0';
    hex_str[1] = 'x';

    /* Extract each hex digit from most significant to least */
    for (int i = 0; i < 8; i++)
    {
        hex_str[2 + i] = hex_digits[(value >> (28 - i * 4)) & 0xF];
    }

    hex_str[10] = '\0';
    vga_puts(hex_str, color);
}

/*
 * get_base_address - Extract base address from GDT entry
 * @entry: Pointer to GDT entry
 * Returns: 32-bit base address
 */
unsigned int get_base_address(struct gdt_entry_t* entry)
{
    unsigned int base = 0;

    base |= (entry->base_low);              /* bits 0-15 */
    base |= (entry->base_middle << 16);     /* bits 16-23 */
    base |= (entry->base_high << 24);       /* bits 24-31 */

    return base;
}

/* =============================================================================
 *                              GDT DISPLAY FUNCTIONS
 * ============================================================================= */

/*
 * gdt_display - Display all GDT entries
 *
 * Shows segment name, address, and access byte for each entry:
 * - Null descriptor
 * - Kernel Code/Data/Stack
 * - User Code/Data/Stack
 */
void gdt_display(void)
{
    char *segments[9] =
    {
        "Null",
        "Kernel Code",
        "Kernel Data",
        "Kernel Stack",
        "User Code",
        "User Data",
        "User Stack"
    };

    vga_puts("---- GDT Registers: ----", RED);
    vga_newline();

    for (int i = 0; i < 7; i++)
    {
        struct gdt_entry_t *entry = gdt + i;

        vga_puts(segments[i], WHITE);
        vga_puts(" address ", GREEN);
        hex_print((unsigned int)&gdt[i], GREEN);
        vga_puts(" | Access: ", YELLOW);
        hex_print(entry->access, YELLOW);
        vga_newline();
    }
}

/*
 * stack_display - Display kernel stack information
 *
 * Shows:
 * - Current stack pointer (ESP)
 * - Stack base address
 * - Current stack usage in bytes
 * - Top 10 values on the stack
 */
void stack_display(void)
{
    int *stack_ptr;
    int stack_base = (int)&kernel_stack_top;
    int value;

    /* Get current stack pointer from ESP register */
    asm volatile("movl %%esp, %0" : "=r"(stack_ptr));

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

    vga_newline();
    vga_puts("---- STACK CONTENT ---- ", RED);
    vga_newline();

    /* Display top 10 stack values */
    for (int i = 0; i < 10 && stack_ptr < (int*)stack_base; i++)
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

/* =============================================================================
 *                              GDT SETUP FUNCTIONS
 * ============================================================================= */

/*
 * gdt_create_entry - Create a GDT descriptor entry
 * @index: Entry index in GDT (0-6)
 * @base: Segment base address
 * @limit: Segment limit (size - 1)
 * @access: Access byte
 * @gran: Granularity and flags
 *
 * The entry is written directly to the GDT at GDT_BASE_ADDRESS
 */
void gdt_create_entry(int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
{
    /* Set base address (split across 3 fields) */
    gdt[index].base_low = (base & 0xFFFF);
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;

    /* Set limit (split across 2 fields) */
    gdt[index].limit_low = (limit & 0xFFFF);
    gdt[index].granularity = ((limit >> 16) & 0x0F);

    /* Set granularity flags and access byte */
    gdt[index].granularity |= gran & 0xF0;
    gdt[index].access = access;
}

/*
 * gdt_init - Initialize the Global Descriptor Table
 *
 * Sets up 7 segment descriptors:
 * [0] Null descriptor (required)
 * [1] Kernel Code: Ring 0, Execute/Read
 * [2] Kernel Data: Ring 0, Read/Write
 * [3] Kernel Stack: Ring 0, Read/Write
 * [4] User Code: Ring 3, Execute/Read
 * [5] User Data: Ring 3, Read/Write
 * [6] User Stack: Ring 3, Read/Write
 *
 * All segments use flat memory model (base=0, limit=4GB)
 */
void gdt_init(void)
{
    /* NULL descriptor (index 0) - required, must be all zeros */
    gdt_create_entry(0, 0, 0, 0, 0);

    /* Kernel Code Segment (offset 0x08)
     * Access: 0x9A = Present | Ring 0 | Code | Execute/Read */
    gdt_create_entry(1, 0, 0xFFFFF, 0x9A, 0xC0);

    /* Kernel Data Segment (offset 0x10)
     * Access: 0x92 = Present | Ring 0 | Data | Read/Write */
    gdt_create_entry(2, 0, 0xFFFFF, 0x92, 0xC0);

    /* Kernel Stack Segment (offset 0x18)
     * Access: 0x92 = Present | Ring 0 | Data | Read/Write */
    gdt_create_entry(3, 0, 0xFFFFF, 0x92, 0xC0);

    /* User Code Segment (offset 0x20)
     * Access: 0xFA = Present | Ring 3 | Code | Execute/Read */
    gdt_create_entry(4, 0, 0xFFFFF, 0xFA, 0xC0);

    /* User Data Segment (offset 0x28)
     * Access: 0xF2 = Present | Ring 3 | Data | Read/Write */
    gdt_create_entry(5, 0, 0xFFFFF, 0xF2, 0xC0);

    /* User Stack Segment (offset 0x30)
     * Access: 0xF2 = Present | Ring 3 | Data | Read/Write */
    gdt_create_entry(6, 0, 0xFFFFF, 0xF2, 0xC0);

    /* Read current GDT pointer using SGDT instruction */
    asm volatile("sgdt %0" : "=m" (gdt_ptr));
    gdt_ptr.base = (unsigned int)gdt;

    /* Display GDT information */
    vga_puts("---- GDT Descriptors---- ", RED);
    vga_newline();

    vga_puts("GDT Base Address: ", WHITE);
    hex_print(gdt_ptr.base, MAGENTA);
    vga_newline();

    vga_puts("GDT Size: ", WHITE);
    hex_print(gdt_ptr.limit, MAGENTA);
    vga_newline();
    vga_newline();
}
