#include "gdt.h"
#include "screen.h"

/* GDT at fixed address 0x800 */
static gdt_entry_t *gdt = (gdt_entry_t*)GDT_ADDRESS;
static gdt_ptr_t gdt_ptr;

/* Create a GDT descriptor entry */
static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[index].base_low = (base & 0xFFFF);
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    
    gdt[index].limit_low = (limit & 0xFFFF);
    gdt[index].granularity = ((limit >> 16) & 0x0F);
    gdt[index].granularity |= (gran & 0xF0);
    gdt[index].access = access;
}

void setup_gdt(void)
{
    /* NULL descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);
    
    /* Kernel Code Segment: 0x08
       Base: 0, Limit: 0xFFFFF
       Access: 0x9A (Present, Ring 0, Code, Executable, Readable) */
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);
    
    /* Kernel Data Segment: 0x10
       Base: 0, Limit: 0xFFFFF
       Access: 0x92 (Present, Ring 0, Data, Writable) */
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);
    
    /* Kernel Stack Segment: 0x18
       Base: 0, Limit: 0xFFFFF
       Access: 0x92 (Present, Ring 0, Data, Writable) */
    gdt_set_entry(3, 0, 0xFFFFF, 0x92, 0xCF);
    
    /* User Code Segment: 0x20
       Base: 0, Limit: 0xFFFFF
       Access: 0xFA (Present, Ring 3, Code, Executable, Readable) */
    gdt_set_entry(4, 0, 0xFFFFF, 0xFA, 0xCF);
    
    /* User Data Segment: 0x28
       Base: 0, Limit: 0xFFFFF
       Access: 0xF2 (Present, Ring 3, Data, Writable) */
    gdt_set_entry(5, 0, 0xFFFFF, 0xF2, 0xCF);
    
    /* User Stack Segment: 0x30
       Base: 0, Limit: 0xFFFFF
       Access: 0xF2 (Present, Ring 3, Data, Writable) */
    gdt_set_entry(6, 0, 0xFFFFF, 0xF2, 0xCF);
    
    /* Get actual GDT info after loading */
    gdt_ptr.limit = (7 * 8) - 1;
    gdt_ptr.base = (uint32_t)gdt;
}

void print_gdt(void)
{
    const char *segments[] = {
        "Null", "Kernel Code", "Kernel Data", "Kernel Stack",
        "User Code", "User Data", "User Stack"
    };
    
    print_str("=== GDT Information ===", CYAN);
    print_new_line();
    print_str("GDT Base: ", WHITE);
    print_hex((uint32_t)gdt, YELLOW);
    print_str("  Size: ", WHITE);
    print_dec(gdt_ptr.limit + 1, YELLOW);
    print_str(" bytes", WHITE);
    print_new_line();
    print_new_line();
    
    for (int i = 0; i < 7; i++) {
        print_str(segments[i], GREEN);
        print_str(": ", WHITE);
        print_str("Sel=", LIGHT_GREY);
        print_hex(i * 8, WHITE);
        print_str(" Access=", LIGHT_GREY);
        print_hex(gdt[i].access, WHITE);
        print_new_line();
    }
}

void print_kernel_stack(void)
{
    uint32_t *stack_ptr;
    uint32_t stack_base = (uint32_t)&stack_top;
    
    __asm__ volatile("movl %%esp, %0" : "=r"(stack_ptr));
    
    print_str("=== Kernel Stack ===", CYAN);
    print_new_line();
    print_str("Stack Top (Base): ", WHITE);
    print_hex(stack_base, YELLOW);
    print_new_line();
    print_str("Current ESP:      ", WHITE);
    print_hex((uint32_t)stack_ptr, YELLOW);
    print_new_line();
    print_str("Stack Used:       ", WHITE);
    print_dec(stack_base - (uint32_t)stack_ptr, YELLOW);
    print_str(" bytes", WHITE);
    print_new_line();
    print_new_line();
    
    print_str("Stack Contents (top 8 entries):", GREEN);
    print_new_line();
    
    for (int i = 0; i < 8 && stack_ptr < (uint32_t*)stack_base; i++) {
        print_str("  [", LIGHT_GREY);
        print_hex((uint32_t)stack_ptr, LIGHT_CYAN);
        print_str("]: ", LIGHT_GREY);
        print_hex(*stack_ptr, WHITE);
        print_new_line();
        stack_ptr++;
    }
}
