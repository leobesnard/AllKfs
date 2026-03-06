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

int print_new_line(void)
{
    cursor_index += COLUMNS_COUNT - ((cursor_index) % COLUMNS_COUNT);
    return 0;
}

int clear_screen(void)
{
    for (unsigned int i = 0; i < (ROWS_COUNT * COLUMNS_COUNT); i++) {
        screen_buffer[i] = 0;
    }
    cursor_index = 0;
    return 0;
}

// Set cursor to specific X,Y position
void set_cursor_position(int x, int y)
{
    if (x < 0) x = 0;
    if (x >= COLUMNS_COUNT) x = COLUMNS_COUNT - 1;
    if (y < 0) y = 0;
    if (y >= ROWS_COUNT) y = ROWS_COUNT - 1;
    
    cursor_index = (y * COLUMNS_COUNT) + x;
}

// Print string at specific position
void print_str_at(const char *s, unsigned char color, int x, int y)
{
    set_cursor_position(x, y);
    print_str(s, color);
}

// Calculate string length (local helper)
static int str_len(const char *s)
{
    int len = 0;
    while (s[len])
        len++;
    return len;
}

// Print centered string on specific row
void print_centered(const char *s, unsigned char color, int row)
{
    int len = str_len(s);
    int x = (COLUMNS_COUNT - len) / 2;
    if (x < 0) x = 0;
    
    set_cursor_position(x, row);
    print_str(s, color);
}