#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

#define ROWS_COUNT      25
#define COLUMNS_COUNT   80
#define VGA_ADDRESS     0xB8000

extern uint16_t* screen_buffer;
extern uint32_t cursor_index;

/* VGA Colors */
#define BLACK           0
#define BLUE            1
#define GREEN           2
#define CYAN            3
#define RED             4
#define MAGENTA         5
#define BROWN           6
#define LIGHT_GREY      7
#define DARK_GREY       8
#define LIGHT_BLUE      9
#define LIGHT_GREEN     10
#define LIGHT_CYAN      11
#define LIGHT_RED       12
#define LIGHT_MAGENTA   13
#define YELLOW          14
#define WHITE           15

/* Screen functions */
int print_char(char c, uint8_t color);
void print_str(const char *s, uint8_t color);
void print_hex(uint32_t value, uint8_t color);
void print_dec(uint32_t value, uint8_t color);
void print_new_line(void);
int clear_screen(void);
void set_cursor_position(int x, int y);
void update_cursor(void);

/* I/O port functions */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#endif
