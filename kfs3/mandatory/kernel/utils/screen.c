#include "screen.h"

uint16_t* screen_buffer;
uint32_t cursor_index = 0;

int print_char(char c, uint8_t color)
{
    if (c == '\n') {
        print_new_line();
        return 0;
    }
    
    if (c >= 32 && c <= 126) {
        screen_buffer[cursor_index] = c | ((uint16_t)color << 8);
        cursor_index++;
        
        // Handle line wrap
        if (cursor_index >= ROWS_COUNT * COLUMNS_COUNT) {
            // Scroll screen up
            for (uint32_t i = 0; i < (ROWS_COUNT - 1) * COLUMNS_COUNT; i++) {
                screen_buffer[i] = screen_buffer[i + COLUMNS_COUNT];
            }
            // Clear last line
            for (uint32_t i = (ROWS_COUNT - 1) * COLUMNS_COUNT; i < ROWS_COUNT * COLUMNS_COUNT; i++) {
                screen_buffer[i] = ' ' | ((uint16_t)WHITE << 8);
            }
            cursor_index = (ROWS_COUNT - 1) * COLUMNS_COUNT;
        }
        
        update_cursor();
    }
    
    return 0;
}

void print_str(const char *s, uint8_t color)
{
    while (*s) {
        print_char(*s, color);
        s++;
    }
}

void print_hex(uint32_t value, uint8_t color)
{
    const char hex_digits[] = "0123456789ABCDEF";
    char hex_str[11];
    
    hex_str[0] = '0';
    hex_str[1] = 'x';
    
    for (int i = 0; i < 8; i++) {
        hex_str[2 + i] = hex_digits[(value >> (28 - i * 4)) & 0xF];
    }
    
    hex_str[10] = '\0';
    print_str(hex_str, color);
}

void print_dec(uint32_t value, uint8_t color)
{
    char dec_str[12];
    int i = 10;
    
    dec_str[11] = '\0';
    
    if (value == 0) {
        print_char('0', color);
        return;
    }
    
    while (value > 0 && i >= 0) {
        dec_str[i--] = '0' + (value % 10);
        value /= 10;
    }
    
    print_str(&dec_str[i + 1], color);
}

void print_new_line(void)
{
    uint32_t offset = COLUMNS_COUNT - (cursor_index % COLUMNS_COUNT);
    cursor_index += offset;
    
    // Handle scroll if needed
    if (cursor_index >= ROWS_COUNT * COLUMNS_COUNT) {
        for (uint32_t i = 0; i < (ROWS_COUNT - 1) * COLUMNS_COUNT; i++) {
            screen_buffer[i] = screen_buffer[i + COLUMNS_COUNT];
        }
        for (uint32_t i = (ROWS_COUNT - 1) * COLUMNS_COUNT; i < ROWS_COUNT * COLUMNS_COUNT; i++) {
            screen_buffer[i] = ' ' | ((uint16_t)WHITE << 8);
        }
        cursor_index = (ROWS_COUNT - 1) * COLUMNS_COUNT;
    }
    
    update_cursor();
}

int clear_screen(void)
{
    for (uint32_t i = 0; i < ROWS_COUNT * COLUMNS_COUNT; i++) {
        screen_buffer[i] = ' ' | ((uint16_t)WHITE << 8);
    }
    cursor_index = 0;
    update_cursor();
    return 0;
}

void set_cursor_position(int x, int y)
{
    if (x < 0) x = 0;
    if (x >= COLUMNS_COUNT) x = COLUMNS_COUNT - 1;
    if (y < 0) y = 0;
    if (y >= ROWS_COUNT) y = ROWS_COUNT - 1;
    
    cursor_index = (y * COLUMNS_COUNT) + x;
    update_cursor();
}

void update_cursor(void)
{
    uint16_t pos = cursor_index;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
