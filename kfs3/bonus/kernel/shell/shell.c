/*
 * Shell Command Handler - KFS-3 Bonus
 * Memory debugging and shell commands
 */

#include "screen.h"
#include "str.h"
#include "gdt.h"
#include "memory.h"

/* Test allocation pointers for shell */
static void *test_ptrs[10] = {NULL};
static int test_count = 0;

/* Get command from screen buffer */
static void get_command(char *cmd, int max_len)
{
    int cmd_start_index = 1;
    char cmdcpy[CMD_BUFFER_SIZE + 1];
    
    while (cmd_start_index <= CMD_BUFFER_SIZE && 
           stock[screen_index][cursor_index - cmd_start_index] != (' ' | ((uint16_t)GREEN << 8))) {
        cmdcpy[CMD_BUFFER_SIZE - cmd_start_index] = 
            *((char *)(&(stock[screen_index][cursor_index - cmd_start_index])));
        cmd_start_index++;
    }
    cmdcpy[CMD_BUFFER_SIZE] = 0;
    
    kstrcpy(cmd, cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1);
}

/* Parse address from command */
static uint32_t parse_address(const char *cmd, int offset)
{
    while (cmd[offset] == ' ') offset++;
    return katoi_hex(&cmd[offset]);
}

/* Parse size from command */
static uint32_t parse_size(const char *cmd, int offset)
{
    while (cmd[offset] == ' ') offset++;
    
    /* Skip first argument */
    while (cmd[offset] && cmd[offset] != ' ') offset++;
    while (cmd[offset] == ' ') offset++;
    
    if (cmd[offset] == '0' && (cmd[offset+1] == 'x' || cmd[offset+1] == 'X')) {
        return katoi_hex(&cmd[offset]);
    }
    return katoi(&cmd[offset]);
}

/* Print help */
static void print_help(void)
{
    print_str("=== KFS-3 Shell Commands ===", CYAN);
    print_new_line();
    print_str("  help        - Show this help", WHITE);
    print_new_line();
    print_str("  clear       - Clear screen", WHITE);
    print_new_line();
    print_str("  gdt         - Display GDT information", WHITE);
    print_new_line();
    print_str("  stack       - Display kernel stack", WHITE);
    print_new_line();
    print_str("  halt        - Halt the CPU", WHITE);
    print_new_line();
    print_str("  reboot      - Reboot the system", WHITE);
    print_new_line();
    print_str("  shutdown    - Shutdown (QEMU only)", WHITE);
    print_new_line();
    print_new_line();
    print_str("=== Memory Commands ===", CYAN);
    print_new_line();
    print_str("  meminfo     - Display memory info", WHITE);
    print_new_line();
    print_str("  pmm         - Physical memory info", WHITE);
    print_new_line();
    print_str("  vmm         - Virtual memory info", WHITE);
    print_new_line();
    print_str("  heap        - Kernel heap info", WHITE);
    print_new_line();
    print_str("  paging      - Paging info", WHITE);
    print_new_line();
    print_str("  memdump <addr> [size] - Dump memory", WHITE);
    print_new_line();
    print_str("  malloc <size>  - Test allocation", WHITE);
    print_new_line();
    print_str("  free           - Free last allocation", WHITE);
    print_new_line();
    print_str("  memtest     - Run memory tests", WHITE);
    print_new_line();
}

int exec_cmd(void)
{
    char cmd[CMD_BUFFER_SIZE + 1];
    get_command(cmd, CMD_BUFFER_SIZE);
    
    if (kstrlen(cmd) == 0) {
        return 0;
    }
    
    print_new_line();
    
    /* Help */
    if (!kstrcmp(cmd, "help")) {
        print_help();
        return 1;
    }
    
    /* Clear screen */
    if (!kstrcmp(cmd, "clear")) {
        clear_screen();
        return 1;
    }
    
    /* GDT info */
    if (!kstrcmp(cmd, "gdt")) {
        print_gdt();
        return 1;
    }
    
    /* Stack info */
    if (!kstrcmp(cmd, "stack")) {
        print_kernel_stack();
        return 1;
    }
    
    /* Halt */
    if (!kstrcmp(cmd, "halt")) {
        print_str("Halting CPU...", YELLOW);
        halt();
        return 1;
    }
    
    /* Reboot */
    if (!kstrcmp(cmd, "reboot")) {
        print_str("Rebooting...", YELLOW);
        reboot();
        return 1;
    }
    
    /* Shutdown */
    if (!kstrcmp(cmd, "shutdown")) {
        print_str("Shutting down...", YELLOW);
        shutdown();
        return 1;
    }
    
    /* Memory info */
    if (!kstrcmp(cmd, "meminfo")) {
        memory_print_info();
        return 1;
    }
    
    /* PMM info */
    if (!kstrcmp(cmd, "pmm")) {
        print_str("=== Physical Memory Manager ===", CYAN);
        print_new_line();
        print_str("Total:  ", WHITE);
        print_dec(pmm_get_total_memory() / 1024 / 1024, YELLOW);
        print_str(" MB (", LIGHT_GREY);
        print_dec(pmm_get_total_pages(), WHITE);
        print_str(" pages)", LIGHT_GREY);
        print_new_line();
        print_str("Used:   ", WHITE);
        print_dec(pmm_get_used_memory() / 1024, YELLOW);
        print_str(" KB (", LIGHT_GREY);
        print_dec(pmm_get_used_pages(), WHITE);
        print_str(" pages)", LIGHT_GREY);
        print_new_line();
        print_str("Free:   ", WHITE);
        print_dec(pmm_get_free_memory() / 1024 / 1024, YELLOW);
        print_str(" MB", WHITE);
        print_new_line();
        return 1;
    }
    
    /* VMM info */
    if (!kstrcmp(cmd, "vmm")) {
        vmm_print_info();
        return 1;
    }
    
    /* Heap info */
    if (!kstrcmp(cmd, "heap")) {
        kheap_print_info();
        return 1;
    }
    
    /* Paging info */
    if (!kstrcmp(cmd, "paging")) {
        paging_print_info();
        return 1;
    }
    
    /* Memory dump */
    if (cmd[0] == 'm' && cmd[1] == 'e' && cmd[2] == 'm' && 
        cmd[3] == 'd' && cmd[4] == 'u' && cmd[5] == 'm' && cmd[6] == 'p') {
        uint32_t addr = parse_address(cmd, 7);
        uint32_t size = parse_size(cmd, 7);
        if (size == 0) size = 64;
        if (size > 256) size = 256;
        
        if (addr == 0) {
            print_str("Usage: memdump <address> [size]", RED);
            print_new_line();
            print_str("Example: memdump 0x100000 32", LIGHT_GREY);
            print_new_line();
        } else {
            memory_dump(addr, size);
        }
        return 1;
    }
    
    /* Malloc test */
    if (cmd[0] == 'm' && cmd[1] == 'a' && cmd[2] == 'l' && 
        cmd[3] == 'l' && cmd[4] == 'o' && cmd[5] == 'c') {
        uint32_t size = parse_address(cmd, 6);
        if (size == 0) size = 64;
        
        if (test_count >= 10) {
            print_str("Max 10 test allocations. Use 'free' first.", RED);
            print_new_line();
        } else {
            void *ptr = kmalloc(size);
            if (ptr) {
                test_ptrs[test_count++] = ptr;
                print_str("Allocated ", GREEN);
                print_dec(size, YELLOW);
                print_str(" bytes at ", WHITE);
                print_hex((uint32_t)ptr, CYAN);
                print_new_line();
                print_str("Actual size: ", WHITE);
                print_dec(ksize(ptr), YELLOW);
                print_str(" bytes", WHITE);
                print_new_line();
            } else {
                print_str("Allocation failed!", RED);
                print_new_line();
            }
        }
        return 1;
    }
    
    /* Free test */
    if (!kstrcmp(cmd, "free")) {
        if (test_count == 0) {
            print_str("No test allocations to free", YELLOW);
            print_new_line();
        } else {
            test_count--;
            void *ptr = test_ptrs[test_count];
            size_t size = ksize(ptr);
            kfree(ptr);
            test_ptrs[test_count] = NULL;
            print_str("Freed allocation at ", GREEN);
            print_hex((uint32_t)ptr, CYAN);
            print_str(" (", LIGHT_GREY);
            print_dec(size, WHITE);
            print_str(" bytes)", LIGHT_GREY);
            print_new_line();
        }
        return 1;
    }
    
    /* Memory test */
    if (!kstrcmp(cmd, "memtest")) {
        memory_test_allocations();
        return 1;
    }
    
    /* Unknown command */
    print_str("Unknown command: ", RED);
    print_str(cmd, YELLOW);
    print_new_line();
    print_str("Type 'help' for available commands", LIGHT_GREY);
    print_new_line();
    
    return 0;
}
