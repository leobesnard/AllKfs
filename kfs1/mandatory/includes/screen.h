#ifndef SCREEN_H
# define SCREEN_H

# define ROWS_COUNT (25)
# define COLUMNS_COUNT (80)

extern unsigned short* screen_buffer;   // array representing the screen 25 (rows) * 80 (columns) * 2 (char + color) = 4000 bytes
extern unsigned int cursor_index;       // current positon in the array

# define VGA_ADDRESS 0xB8000 // adress of the array representing the screen

# define BLACK 0
# define GREEN 2
# define RED 4
# define YELLOW 14
# define WHITE 15 //  00001111 (foreground white, background black)

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
int print_str(const char *s, unsigned char color);
int print_new_line(void);
int clear_screen(void);

// Helper functions for positioning
void set_cursor_position(int x, int y);
void print_str_at(const char *s, unsigned char color, int x, int y);
void print_centered(const char *s, unsigned char color, int row);

// String utility functions (from str.c)
int kstrcmp(const char *s1, const char *s2);
int kstrlen(const char *s);

#endif