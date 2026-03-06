#include "gdt.h"
#include "screen.h"

// int stack_space;
// Structure pour un descripteur GDT
struct GDTEntry {
    unsigned short limit_low;     // Limite (bits 0-15)
    unsigned short base_low;      // Base (bits 0-15)
    unsigned char  base_middle;   // Base (bits 16-23)
    unsigned char  access;        // Octet d'accès
    unsigned char  granularity;   // Granularité et limite (bits 16-19)
    unsigned char  base_high;     // Base (bits 24-31)
} __attribute__((packed));

struct {
    unsigned short limit;
    unsigned int base;
} gdt_ptr;

// GDT avec 6 entrées (NULL + 6 segments requis)
struct GDTEntry *gdt = (struct GDTEntry*)GDT_ADRESS;

// Fonction pour afficher un entier en hexadécimal
void print_hex(unsigned int value, unsigned char color) {
    const char hex_digits[] = "0123456789ABCDEF";
    char hex_str[11]; // 0x + 8 chiffres + '\0'
    
    hex_str[0] = '0';
    hex_str[1] = 'x';
    
    for (int i = 0; i < 8; i++) {
        hex_str[2 + i] = hex_digits[(value >> (28 - i * 4)) & 0xF];  // Décale les bits pour chaque hex
         // hex_str[9 - i] = hex_digits[value & 0xF];
        // value >>= 4;
    }
    
    hex_str[10] = '\0';
    print_str(hex_str, color);
}

unsigned int get_base_address(struct GDTEntry* entry) {
    unsigned int base = 0;
    base |= (entry->base_low);             // bits 0-15
    base |= (entry->base_middle << 16);   // bits 16-23
    base |= (entry->base_high << 24);     // bits 24-31
    return base;
}

void print_gdt() {
    
    char *segments[9] = {"Null", "Kernel Code", "Kernel Data", "Kernel Stack", "User Code", "User Data", "User Stack"};
    
    print_str("---- GDT Registres: ----", RED);
    print_new_line();
    for (int i = 0; i < 7; i++) {
        struct GDTEntry *entry = gdt + i;
        print_str(segments[i], WHITE);
        print_str(" adress ", GREEN);
        print_hex((unsigned int)&gdt[i], GREEN);
        print_str(" | Access: ", YELLOW);
        print_hex(entry->access, YELLOW);
        print_new_line();
    }
}

// Fonction pour afficher le contenu de la pile kernel
void print_kernel_stack() {

    int *stack_ptr;
    int stack_base = (int)&stack_space;
    int value;
 
    // Récupère le pointeur de pile actuel
    asm volatile("movl %%esp, %0" : "=r"(stack_ptr));

    print_new_line();
    print_str("---- KERNEL STACK ---- ", RED);
    print_new_line();
    print_str("Stack Address: ", WHITE);
    print_hex((unsigned int)stack_ptr, LIGHT_CYAN);
    print_new_line();
    print_str("Stack Base: ", WHITE);
    print_hex(stack_base, LIGHT_CYAN);
    print_new_line();
    print_str("Stack Size: ", WHITE);
    print_hex(stack_base - (unsigned int)stack_ptr, LIGHT_CYAN);
    print_str(" bytes", LIGHT_CYAN);
    print_new_line();

    print_new_line();
    print_str("---- STACK CONTENT ---- ", RED); // top 10 values
    print_new_line();

    for (int i = 0; i < 4 && stack_ptr < (int*)stack_base; i++) {
        print_str("Stack[", LIGHT_GREEN);
        print_hex((unsigned int)stack_ptr, LIGHT_GREEN);
        print_str("]: ", LIGHT_GREEN);
        value = *stack_ptr;
        print_hex(value, WHITE);
        stack_ptr++;
        print_new_line();
        // print_str(" | ", RED);
    }
}

// Fonction pour configurer un descripteur GDT
void create_descriptor(int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran) {
    // Définir la base
    gdt[index].base_low = (base & 0xFFFF);                    // 16 bits de base
    gdt[index].base_middle = (base >> 16) & 0xFF;             // 8 bits de base
    gdt[index].base_high = (base >> 24) & 0xFF;               // 8 bits de base

    // Définir la limite
    gdt[index].limit_low = (limit & 0xFFFF);                  // 16 bits de limite
    gdt[index].granularity = ((limit >> 16) & 0x0F);          // 4 bits de granularité

    // Définir la granularité et l'accès
    gdt[index].granularity |= gran & 0xF0;                     // Paramètre de granularité
    gdt[index].access = access;                                // Accès
}

// Fonction pour initialiser la GDT
void setup_gdt() {
       // Structure pour le pointeur GDT à récupérer avec sgdt
    


    
    // NULL descriptor
    create_descriptor(0, 0, 0, 0, 0);
    
    // Kernel Code Segment (Offset: 0x08) / Base: 0, Limit: 0xFFFFF, Access: 0x9A 
    create_descriptor(1, 0, 0xFFFFF, 0x9A, 0xC0);
    
    // Kernel Data Segment (Offset: 0x10) / Base: 0, Limit: 0xFFFFF, Access: 0x92 (Present, Ring 0, Data, Writable) exemple dans README
    create_descriptor(2, 0, 0xFFFFF, 0x92, 0xC0);
    
    // Kernel Stack Segment (Offset: 0x18) / Base: 0, Limit: 0xFFFFF, Access: 0x92 
    create_descriptor(3, 0, 0xFFFFF, 0x92, 0xC0);
    
    // User Code Segment (Offset: 0x20) / Base: 0, Limit: 0xFFFFF, Access: 0xFA 
    create_descriptor(4, 0, 0xFFFFF, 0xFA, 0xC0);
    
    // User Data Segment (Offset: 0x28) / Base: 0, Limit: 0xFFFFF, Access: 0xF2 
    create_descriptor(5, 0, 0xFFFFF, 0xF2, 0xC0);
    
    // User Stack Segment (Offset: 0x30) / Base: 0, Limit: 0xFFFFF, Access: 0xF2 
    create_descriptor(6, 0, 0xFFFFF, 0xF2, 0xC0);

  // Récupère la base de la GDT et la limite avec l'instruction SGDT
    asm volatile("sgdt %0" : "=m" (gdt_ptr));
    // asm volatile ("sgdt %0" : : "m"(gdt_ptr));
    // gdt_ptr.limit = sizeof(*gdt);
    gdt_ptr.base = (unsigned int)gdt;


    print_str("---- GDT Descriptors---- ", RED);
    print_new_line();
    print_str("GDT Base Address: ", WHITE);
    // kprintf("%x", gdt_ptr.base);
    print_hex(gdt_ptr.base, MAGENTA);
    print_new_line();
    print_str("GDT Size: ", WHITE);
    print_hex(gdt_ptr.limit, MAGENTA); // 56 bytes = 7 registres * 8 octets
    print_new_line();
    print_new_line();



}