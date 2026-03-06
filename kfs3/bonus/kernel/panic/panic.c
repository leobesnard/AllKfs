/*
 * Kernel Panic Handler - Bonus Version
 */

#include "panic.h"
#include "screen.h"

static const char* panic_types[] = {
    "General Kernel Panic",
    "Out of Memory",
    "Page Fault",
    "Invalid Pointer",
    "Double Free",
    "Heap Corruption",
    "Assertion Failed"
};

static void halt_cpu(void)
{
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}

void kernel_panic(const char *message, const char *file, uint32_t line, int type)
{
    __asm__ volatile("cli");
    
    screen_buffer = (uint16_t*)VGA_ADDRESS;
    for (uint32_t i = 0; i < ROWS_COUNT * COLUMNS_COUNT; i++) {
        screen_buffer[i] = ' ' | (RED << 12) | (WHITE << 8);
    }
    cursor_index = 0;
    
    print_new_line();
    print_str("  !!! KERNEL PANIC !!!", WHITE);
    print_new_line();
    print_new_line();
    
    print_str("  Type: ", YELLOW);
    if (type >= 0 && type <= PANIC_ASSERTION_FAILED) {
        print_str(panic_types[type], WHITE);
    } else {
        print_str("Unknown", WHITE);
    }
    print_new_line();
    print_new_line();
    
    print_str("  Message: ", YELLOW);
    print_str(message, WHITE);
    print_new_line();
    print_new_line();
    
    print_str("  File: ", YELLOW);
    print_str(file, WHITE);
    print_new_line();
    print_str("  Line: ", YELLOW);
    print_dec(line, WHITE);
    print_new_line();
    print_new_line();
    
    uint32_t eax, ebx, ecx, edx, esp, ebp, esi, edi;
    __asm__ volatile(
        "movl %%eax, %0\n"
        "movl %%ebx, %1\n"
        "movl %%ecx, %2\n"
        "movl %%edx, %3\n"
        "movl %%esp, %4\n"
        "movl %%ebp, %5\n"
        "movl %%esi, %6\n"
        "movl %%edi, %7\n"
        : "=m"(eax), "=m"(ebx), "=m"(ecx), "=m"(edx),
          "=m"(esp), "=m"(ebp), "=m"(esi), "=m"(edi)
    );
    
    print_str("  Registers:", CYAN);
    print_new_line();
    print_str("    EAX=", LIGHT_GREY);
    print_hex(eax, WHITE);
    print_str("  EBX=", LIGHT_GREY);
    print_hex(ebx, WHITE);
    print_new_line();
    print_str("    ESP=", LIGHT_GREY);
    print_hex(esp, WHITE);
    print_str("  EBP=", LIGHT_GREY);
    print_hex(ebp, WHITE);
    print_new_line();
    print_new_line();
    
    print_str("  System Halted.", RED);
    halt_cpu();
}

void kernel_warning(const char *message, const char *file, uint32_t line)
{
    print_str("[WARNING] ", YELLOW);
    print_str(message, WHITE);
    print_str(" (", LIGHT_GREY);
    print_str(file, WHITE);
    print_str(":", LIGHT_GREY);
    print_dec(line, WHITE);
    print_str(")", LIGHT_GREY);
    print_new_line();
}
