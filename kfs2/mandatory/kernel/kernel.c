#include "screen.h"
#include "gdt.h"

void main() {
    // Initialiser l'écran
    screen_buffer = (unsigned short *)VGA_ADDRESS;
    clear_screen();
    // Afficher informations GDT 
    setup_gdt();
    print_gdt();
    // Afficher kernel stack
    print_kernel_stack();
    print_new_line();
}
