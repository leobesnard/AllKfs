/*
 * Paging Implementation - Bonus Version
 */

#include "memory.h"
#include "screen.h"
#include "panic.h"

static page_directory_t page_directory __attribute__((aligned(PAGE_SIZE)));
static uint32_t kernel_page_tables[4][1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t current_page_directory = 0;
static int paging_enabled = 0;

static inline uint32_t pde_index(uint32_t virtual_addr) { return (virtual_addr >> 22) & 0x3FF; }
static inline uint32_t pte_index(uint32_t virtual_addr) { return (virtual_addr >> 12) & 0x3FF; }

static inline void tlb_flush(uint32_t virtual_addr)
{
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

static inline void load_page_directory(uint32_t pd_physical)
{
    __asm__ volatile("movl %0, %%cr3" : : "r"(pd_physical) : "memory");
}

static inline void enable_paging_cr0(void)
{
    uint32_t cr0;
    __asm__ volatile("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("movl %0, %%cr0" : : "r"(cr0) : "memory");
}

void paging_init(void)
{
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }
    
    for (int table = 0; table < 4; table++) {
        for (int page = 0; page < 1024; page++) {
            uint32_t physical_addr = (table * 1024 + page) * PAGE_SIZE;
            kernel_page_tables[table][page] = physical_addr | PTE_PRESENT | PTE_WRITABLE;
        }
        page_directory[table] = (uint32_t)kernel_page_tables[table] | PTE_PRESENT | PTE_WRITABLE;
    }
    
    current_page_directory = (uint32_t)page_directory;
}

void paging_enable(void)
{
    if (paging_enabled) return;
    load_page_directory(current_page_directory);
    enable_paging_cr0();
    paging_enabled = 1;
}

static uint32_t* get_page_table(uint32_t virtual_addr, int create)
{
    uint32_t pd_index = pde_index(virtual_addr);
    
    if (page_directory[pd_index] & PTE_PRESENT) {
        return (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    }
    
    if (!create) return NULL;
    
    uint32_t *new_table = (uint32_t*)pmm_alloc_page();
    if (new_table == NULL) return NULL;
    
    memset(new_table, 0, PAGE_SIZE);
    page_directory[pd_index] = (uint32_t)new_table | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    
    return new_table;
}

void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags)
{
    virtual_addr &= 0xFFFFF000;
    physical_addr &= 0xFFFFF000;
    
    uint32_t *page_table = get_page_table(virtual_addr, 1);
    if (page_table == NULL) {
        PANIC("Failed to get page table for mapping");
    }
    
    uint32_t pt_index = pte_index(virtual_addr);
    page_table[pt_index] = physical_addr | flags | PTE_PRESENT;
    
    if (paging_enabled) {
        tlb_flush(virtual_addr);
    }
}

void paging_unmap_page(uint32_t virtual_addr)
{
    virtual_addr &= 0xFFFFF000;
    
    uint32_t *page_table = get_page_table(virtual_addr, 0);
    if (page_table == NULL) return;
    
    uint32_t pt_index = pte_index(virtual_addr);
    page_table[pt_index] = 0;
    
    if (paging_enabled) {
        tlb_flush(virtual_addr);
    }
}

uint32_t paging_get_physical(uint32_t virtual_addr)
{
    uint32_t *page_table = get_page_table(virtual_addr, 0);
    if (page_table == NULL) return 0;
    
    uint32_t pt_index = pte_index(virtual_addr);
    
    if (!(page_table[pt_index] & PTE_PRESENT)) return 0;
    
    return (page_table[pt_index] & 0xFFFFF000) | (virtual_addr & 0xFFF);
}

page_table_t paging_alloc_table(void)
{
    uint32_t *table = (uint32_t*)pmm_alloc_page();
    if (table) memset(table, 0, PAGE_SIZE);
    return table;
}

void paging_print_info(void)
{
    print_str("=== Paging Information ===", CYAN);
    print_new_line();
    print_str("Page Directory: ", WHITE);
    print_hex(current_page_directory, YELLOW);
    print_new_line();
    print_str("Status: ", WHITE);
    print_str(paging_enabled ? "ENABLED" : "DISABLED", paging_enabled ? GREEN : RED);
    print_new_line();
    
    int mapped_tables = 0;
    int mapped_pages = 0;
    
    for (int i = 0; i < 1024; i++) {
        if (page_directory[i] & PTE_PRESENT) {
            mapped_tables++;
            uint32_t *pt = (uint32_t*)(page_directory[i] & 0xFFFFF000);
            for (int j = 0; j < 1024; j++) {
                if (pt[j] & PTE_PRESENT) mapped_pages++;
            }
        }
    }
    
    print_str("Page Tables: ", WHITE);
    print_dec(mapped_tables, YELLOW);
    print_new_line();
    print_str("Mapped Pages: ", WHITE);
    print_dec(mapped_pages, YELLOW);
    print_str(" (", LIGHT_GREY);
    print_dec(mapped_pages * 4, WHITE);
    print_str(" KB)", LIGHT_GREY);
    print_new_line();
}
