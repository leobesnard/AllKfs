#ifndef GDT_H
# define GDT_H
# define GDT_ADRESS 0x800
extern int stack_space;

void print_gdt();
void print_kernel_stack();
void print_hex(unsigned int value, unsigned char color);
void create_descriptor(int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran);
void setup_gdt();


#endif