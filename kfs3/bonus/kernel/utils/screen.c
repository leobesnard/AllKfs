/*
 * Screen utilities with multiple screen support for KFS-3 Bonus
 */

#include "screen.h"
#include "str.h"
#include "gdt.h"
#include "memory.h"

uint16_t* screen_buffer;
uint32_t cursor_index = 0;
int screen_index = 0;
uint16_t stock[SCREEN_COUNT][BUFFER_ROW_COUNT * COLUMNS_COUNT];
uint16_t stock_cursor_index[SCREEN_COUNT];
uint16_t extra_scroll[SCREEN_COUNT];
uint32_t total_row[SCREEN_COUNT];

int print_char(char c, uint8_t color)
{
    extra_scroll[screen_index] = 0;
    
    if (c == '\n') {
        print_new_line();
        return 0;
    }
    
    if (c >= 32 && c <= 126) {
        if (cursor_index < ROWS_COUNT * COLUMNS_COUNT) {
            screen_buffer[cursor_index] = c | ((uint16_t)color << 8);
        }
        stock[screen_index][cursor_index] = c | ((uint16_t)color << 8);
        cursor_index++;
    }
    
    if (cursor_index % COLUMNS_COUNT == 0) {
        if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1)) {
            for (uint32_t i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
            }
            for (uint32_t i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i] = ' ' | ((uint16_t)BLACK << 8);
            }
            cursor_index -= COLUMNS_COUNT;
        } else {
            total_row[screen_index]++;
        }
    }
    
    if (total_row[screen_index] >= ROWS_COUNT) {
        scroll_screen();
    } else {
        update_cursor();
    }
    
    return 0;
}

void delete_char(void)
{
    if (cursor_index == 0)
        return;
    if (stock[screen_index][cursor_index - 1] == (' ' | ((uint16_t)GREEN << 8))) {
        return;  // Don't delete past prompt
    }
    
    cursor_index--;
    if (cursor_index < ROWS_COUNT * COLUMNS_COUNT) {
        screen_buffer[cursor_index] = ' ' | ((uint16_t)BLACK << 8);
    }
    stock[screen_index][cursor_index] = ' ' | ((uint16_t)BLACK << 8);
    
    if (total_row[screen_index] >= ROWS_COUNT) {
        scroll_screen();
    } else {
        update_cursor();
    }
}

void print_str(const char *s, uint8_t color)
{
    while (*s) {
        if (*s == '\n') {
            print_new_line();
        } else {
            print_char(*s, color);
        }
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

void print_hex_short(uint32_t value, uint8_t color)
{
    const char hex_digits[] = "0123456789ABCDEF";
    char hex_str[5];
    
    for (int i = 0; i < 4; i++) {
        hex_str[i] = hex_digits[(value >> (12 - i * 4)) & 0xF];
    }
    hex_str[4] = '\0';
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
    for (; offset > 0; offset--) {
        stock[screen_index][cursor_index] = ' ' | ((uint16_t)BLACK << 8);
        cursor_index++;
    }
    
    if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1)) {
        for (uint32_t i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
            stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
        }
        for (uint32_t i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
            stock[screen_index][i] = ' ' | ((uint16_t)BLACK << 8);
        }
        cursor_index -= COLUMNS_COUNT;
    } else {
        total_row[screen_index]++;
    }
    
    display_screen(screen_index);
}

int clear_screen(void)
{
    cursor_index = 0;
    total_row[screen_index] = 0;
    extra_scroll[screen_index] = 0;
    
    for (uint32_t i = 0; i < (BUFFER_ROW_COUNT * COLUMNS_COUNT); i++) {
        stock[screen_index][i] = ' ' | ((uint16_t)BLACK << 8);
    }
    
    for (uint32_t i = 0; i < ROWS_COUNT * COLUMNS_COUNT; i++) {
        screen_buffer[i] = ' ' | ((uint16_t)BLACK << 8);
    }
    
    cursor_index = 0;
    total_row[screen_index] = 0;
    update_cursor();
    return 0;
}

void set_cursor_offset(int offset)
{
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(offset & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((offset >> 8) & 0xFF));
}

void update_cursor(void)
{
    set_cursor_offset(cursor_index);
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

void scroll_screen(void)
{
    uint32_t overflow_rows = total_row[screen_index] - ROWS_COUNT + 1;
    
    if (overflow_rows > 0) {
        overflow_rows = extra_scroll[screen_index] > overflow_rows ? 0 : overflow_rows - extra_scroll[screen_index];
    }
    
    uint32_t start = overflow_rows * COLUMNS_COUNT;
    int last_char_index = 0;
    
    for (uint32_t i = 0; i < (ROWS_COUNT * COLUMNS_COUNT); i++) {
        screen_buffer[i] = stock[screen_index][start + i] ? 
                           stock[screen_index][start + i] : 
                           ' ' | ((uint16_t)BLACK << 8);
        if (screen_buffer[i] && screen_buffer[i] != (' ' | ((uint16_t)BLACK << 8))) {
            last_char_index = i;
        }
    }
    
    set_cursor_offset(last_char_index + 1);
}

void display_screen(int screen_idx)
{
    if (screen_idx < 0 || screen_idx >= SCREEN_COUNT)
        return;
    
    if (total_row[screen_idx] >= ROWS_COUNT) {
        scroll_screen();
    } else {
        int last_char_index = 0;
        for (uint32_t u = 0; u < ROWS_COUNT * COLUMNS_COUNT; u++) {
            screen_buffer[u] = stock[screen_idx][u];
            if (screen_buffer[u] && screen_buffer[u] != (' ' | ((uint16_t)BLACK << 8))) {
                last_char_index = u;
            }
        }
        set_cursor_offset(last_char_index + 1);
    }
}

void switch_screen(int new_screen_index)
{
    if (new_screen_index < 0 || new_screen_index >= SCREEN_COUNT)
        return;
    
    stock_cursor_index[screen_index] = cursor_index;
    screen_index = new_screen_index;
    cursor_index = stock_cursor_index[screen_index];
    display_screen(screen_index);
}

void new_prompt(void)
{
    print_new_line();
    print_str("kfs3", LIGHT_CYAN);
    print_str("@", GREEN);
    print_str("screen", WHITE);
    print_dec(screen_index + 1, YELLOW);
    print_str("> ", GREEN);
}

void halt(void)
{
    __asm__ volatile("cli; hlt");
}

void reboot(void)
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    halt();
}

void shutdown(void)
{
    outw(0x604, 0x2000);
}
