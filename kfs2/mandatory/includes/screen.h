#ifndef SCREEN_H
# define SCREEN_H

# define ROWS_COUNT (25)
# define COLUMNS_COUNT (80)

extern unsigned short* screen_buffer;   // array representing the screen 25 (rows) * 80 (columns) * 2 (char + color) = 4000 bytes
extern unsigned int cursor_index;       // current positon in the array


# define VGA_ADDRESS 0xB8000 // adress of the array representing the screen

// Définition des couleurs VGA
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
#define WHITE 15

int print_char(char c, unsigned char color);
int print_str(const char *s, unsigned char color);
int print_new_line();
int clear_screen();

#endif