#include "screen.h"
#include "keyboard.h"
#include "ft_printf.h"

void init_screens()
{
    // mémoire est mappée dans le segment de mémoire à l'adresse 0xB8000
    screen_buffer = (unsigned short *)VGA_ADDRESS;
    for (int i = 0; i < SCREEN_COUNT; i++) {
        switch_screen(i);
        clear_screen();
        print_str("Welcome to screen", GREEN);
        kprintf(" %d ", i + 1);
        print_str("!\n", GREEN);
    }
    switch_screen(0);
}

void switch_screen(int new_screen_index)
{
    if (new_screen_index < 0 || new_screen_index >= SCREEN_COUNT)
        return ;

    stock_cursor_index[screen_index] = cursor_index;
    screen_index = new_screen_index;
    cursor_index = stock_cursor_index[screen_index];
    display_screen(screen_index);
}

void display_screen(int screen_index)
{
    if (screen_index < 0 || screen_index >= SCREEN_COUNT)
        return ;
    if (total_row[screen_index] >= ROWS_COUNT)
    {
        scroll_screen();
    }
    else
    {
        int last_char_index = 0;
        for (int u = 0; u < ROWS_COUNT * COLUMNS_COUNT; u++) {
            screen_buffer[u] = stock[screen_index][u];
            if (screen_buffer[u] && screen_buffer[u] != (' ' | (unsigned short)YELLOW<<8)) {
                last_char_index = u;
            }
        }
        set_cursor_offset(last_char_index + 1);
    } 
}


void main()
{
    set_cursor_offset(0);
    init_screens();
    handle_keyboard();
}


//TO DO
// FACTORISATIO -> -Cursor detection
//                 -total_line++ (buffer overflow)
//                 -empecher l'overwrite de l'enete de page (welcome 1, 2, 3) (buffer overflow) 