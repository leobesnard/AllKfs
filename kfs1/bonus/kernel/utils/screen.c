#include "screen.h"
#include "ft_printf.h"

unsigned short* screen_buffer;
unsigned int cursor_index = 0;
unsigned int total_row[SCREEN_COUNT];
unsigned char scancode = 0;

int print_char(char c, unsigned char color)
{
    extra_scroll[screen_index] = 0;
    if (c == '\n') {
        print_new_line();
        return 0;
    } else if (c >= 32 && c <= 126){
        if (cursor_index < ROWS_COUNT * COLUMNS_COUNT)
            screen_buffer[cursor_index] = c | (unsigned short)color << 8;
        stock[screen_index][cursor_index] = c | (unsigned short)color << 8;
        cursor_index++;
    }
    // le premier octet est le caractère lui-même (selon le code ASCII)
    // le second octet contient des informations sur la couleur de fond et la couleur du texte.
    // Bits 0-3 : Couleur du texte (ex: 0x07 = blanc).
    // Bits 4-7 : Couleur de fond (ex: 0x00 = fond noir).

    
    if (cursor_index % COLUMNS_COUNT == 0)
    {
        if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1))
        {
            for (int i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
            }
            for (int i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
            }
            cursor_index -= COLUMNS_COUNT;
        }
        else
        {
            total_row[screen_index]++;
        }
    }
    if (total_row[screen_index] >= ROWS_COUNT)
    {
        scroll_screen();
    }
    else
    {
        update_cursor();
    }
    
    return 0;
}



void print_str(const char *s, unsigned char color)
{
    unsigned int i = 0;

    while (s[i]) 
    {
        if (s[i] == '\n')
        {
            print_new_line();
        }
        else 
        {
            print_char(s[i], color);
        }
        i++;
    }
}

void print_str_n(const char *s, unsigned char color, unsigned int n)
{
    if (n < 0)
        return;
    unsigned int i = 0;

    while (s[i] && i < n) 
    {
        if (s[i] == '\n')
        {
            print_new_line();
        }
        else 
        {
            print_char(s[i], color);
        }
        i++;
    }
}

void print_new_line()
{

    // if (cursor_index % COLUMNS_COUNT == 0)
    // {
    //     // + 80 = Ligne suivante si debut de ligne
    //     cursor_index += COLUMNS_COUNT;
    // }
    // else
    // {
    int offset = COLUMNS_COUNT - ((cursor_index) % COLUMNS_COUNT);
    for (; offset > 0; offset--) {
        stock[screen_index][cursor_index] = ' ' | (unsigned short)WHITE << 8;
        cursor_index++;
        // print_char(' ', WHITE);
    }
    if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1))
        {
            for (int i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
            }
            for (int i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
            }
            cursor_index -= COLUMNS_COUNT;
        }
        else
        {
            total_row[screen_index]++;
        }
    display_screen(screen_index);
    // }
    // total_row[screen_index]++;
    // if (total_row[screen_index] > ROWS_COUNT)
    // {
    //     scroll_screen();
    // }
    // else 
    // {
    //     update_cursor();
    // }
}

int clear_screen()
{
    cursor_index = 0;
    total_row[screen_index] = 0;
    for (unsigned int i = 0; i < (BUFFER_ROW_COUNT * COLUMNS_COUNT); i++)
    {
        stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
        // print_char(' ', YELLOW);
    }
    cursor_index = 0;
    total_row[screen_index] = 0;
    update_cursor();
    return 0;
}

int kstrlen(char *s)
{
    char *new = s;
    while (*new)
    {
        new++;
    }
    return new - s;
}

static inline void outb(int port, int value) {
    // outb is a macro that writes a byte to the port
    asm volatile ("outb %%al, %%dx" : : "a"(value), "d"(port));
}


void set_cursor_offset(int offset) 
{
    outb(0x3D4, 0x0F); // LOW BYTE
    outb(0x3D5, (uint8_t)(offset & 0xFF));

    outb(0x3D4, 0x0E); // HIGH BYTE
    outb(0x3D5, (uint8_t)((offset >> 8)));
}
void update_cursor() {
    set_cursor_offset(cursor_index);
}

void set_cursor(int x, int y) 
{
    unsigned short position = y * COLUMNS_COUNT + x;
    set_cursor_offset(position);
}
