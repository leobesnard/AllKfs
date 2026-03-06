#include "screen.h"
// Codes standard pour les touches flèches
#define KEY_UP    0x48  // Scancode pour flèche haut
#define KEY_DOWN  0x50  // Scancode pour flèche bas

unsigned short stock[SCREEN_COUNT][ROWS_COUNT * COLUMNS_COUNT*2];   // array representing the screen 25 (rows) * 80 (columns) * 2 (char + color) = 4000 bytes
int screen_index;
unsigned short stock_cursor_index[SCREEN_COUNT];
unsigned short extra_scroll[SCREEN_COUNT];

// static inline unsigned char inb(unsigned short port)
// {
//     unsigned char value;
//     __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
//     return value;
// }

// unsigned char read_scancode()
// {
//     return inb(0x60);  // Lit un octet depuis le port d'E/S 0x60
// }

// void keyboard_interrupt_handler()
// {
//     scancode = read_scancode();
//     // Acquitter l'interruption
//     outb(0x20, 0x20);  // EOI au PIC
// }

void scroll_screen()
{
    unsigned int overflow_rows = total_row[screen_index] - ROWS_COUNT + 1;

    //  switch(scancode) {
    //     case KEY_UP:
    //         // Scroll vers le haut (afficher contenu précédent)
    //         if (overflow_rows > 0) {
    //             overflow_rows--;
    //             scroll_screen();
    //         }
    //         break;
            
    //     case KEY_DOWN:
    //         // Scroll vers le bas (afficher contenu suivant)
    //         if (overflow_rows < total_row) {
    //             overflow_rows++;
    //             scroll_screen();
    //         }
    //         break;
    // }
    if (overflow_rows > 0) {
        overflow_rows = extra_scroll[screen_index] > overflow_rows ? 0 : overflow_rows - extra_scroll[screen_index];
    }
    
    unsigned int start = overflow_rows * COLUMNS_COUNT;
    int last_char_index = 0;
    for (unsigned int i = 0; i < (ROWS_COUNT * COLUMNS_COUNT); i++)
    {
        screen_buffer[i] = stock[screen_index][start + i] ? stock[screen_index][start + i] : ' ' | (unsigned short)WHITE<<8 ;
        if (screen_buffer[i] && screen_buffer[i] != (' ' | (unsigned short)YELLOW<<8)) {
            last_char_index = i;
        }
        // update_cursor();
    }
    set_cursor_offset(last_char_index + 1);
}