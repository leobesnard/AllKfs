#ifndef SCREEN_H
# define SCREEN_H

# define ROWS_COUNT (25)
# define COLUMNS_COUNT (80)

// #include <stdlib.h>


extern unsigned short* screen_buffer;   // array representing the screen 25 (rows) * 80 (columns) * 2 (char + color) = 4000 bytes

#define SCREEN_COUNT 3
#define BUFFER_ROW_COUNT (ROWS_COUNT * 2)
extern unsigned short stock[SCREEN_COUNT][BUFFER_ROW_COUNT * COLUMNS_COUNT];   // array representing the screen 25 (rows) * 80 (columns) * 2 (char + color) = 4000 bytes
extern int screen_index;
extern unsigned short stock_cursor_index[SCREEN_COUNT];
extern unsigned short extra_scroll[SCREEN_COUNT];

extern unsigned int cursor_index;       // current positon in the array
extern unsigned int total_row[SCREEN_COUNT];
extern unsigned char scancode;

#define CMD_BUFFER_SIZE 60

// extern char *buffer;
// extern char *buffer_color;

# define VGA_ADDRESS 0xB8000 // adress of the array representing the screen

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHT_GREY 7
#define DARK_GREY 8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_MAGENTA 13
#define YELLOW 14
#define WHITE 15 //  00001111 (foreground white, background black)

// 7 6 5 4 3 2 1 0
// | | | | | | | |
// | | | | | | | |
// | | | | | | | |
// | | | | +---- Foreground color (4 bits)
// | | | |
// | | | |
// | +---Background color (3 bits)
// +-Reserved (1 bit, often set to 0, can be used for style on some system)

int print_char(char c, unsigned char color);
void print_str(const char *s, unsigned char color);
void print_str_n(const char *s, unsigned char color, unsigned int n);
void print_new_line();
int clear_screen();
void switch_screen(int new_screen_index);
void scroll_screen(void);
void display_screen(int screen_index);
void update_cursor();
void set_cursor(int x, int y);
void set_cursor_offset(int offset);
void new_prompt();
void delete_char();
int exec_cmd();
static inline void outb(int port, int value) {
    // outb is a macro that writes a byte to the port
    asm volatile ("outb %%al, %%dx" : : "a"(value), "d"(port));
}

static inline void outw(int port, int value) {
    // outw écrit un mot (2 octets) dans le port I/O
    asm volatile ("outw %%ax, %%dx" : : "a"(value), "d"(port));
}

static inline unsigned char inb(unsigned short port) {
    // inb is a macro that reads a byte from the port
    unsigned char value;

    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#endif