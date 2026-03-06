#include "screen.h"

unsigned short* screen_buffer;
unsigned int cursor_index = 0;

int print_char(char c, unsigned char color)
{
    screen_buffer[cursor_index] = c | (unsigned short)color << 8;
    cursor_index++;

    // le premier octet est le caractère lui-même (selon le code ASCII)
    // le second octet contient des informations sur la couleur de fond et la couleur du texte.
    // Bits 0-3 : Couleur du texte (ex: 0x07 = blanc).
    // Bits 4-7 : Couleur de fond (ex: 0x00 = fond noir).
    return 0;
}

int print_str(const char *s, unsigned char color)
{
    int index = 0;
    while (s[index]) 
    {
        print_char(s[index], color);
        index++;
    }

    return index;
}

int print_new_line() {
    cursor_index += COLUMNS_COUNT - ((cursor_index) % COLUMNS_COUNT);
    return 0;
}

int clear_screen()
{
    for (unsigned int i = 0; i < (ROWS_COUNT * COLUMNS_COUNT); i++) {
        screen_buffer[i] = 0;
    }
    cursor_index = 0;
    return 0;
}