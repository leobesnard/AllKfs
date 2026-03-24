/* ************************************************************************** */
/*                                                                            */
/*   screen.h - VGA text mode display driver interface                        */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCREEN_H
#define SCREEN_H

/*
** VGA Display Configuration
** Standard text mode: 80 columns x 25 rows
*/
#define VGA_COLS        (80)
#define VGA_ROWS        (25)
#define VGA_MEMORY_BASE 0xB8000

/*
** Color attribute format (1 byte per character):
**
**   Bit:  7   6   5   4   3   2   1   0
**         |   |   |   |   |___|___|___|
**         |   |___|___|       |
**         |       |           +-- Foreground color (4 bits)
**         |       +-------------- Background color (3 bits)
**         +---------------------- Blink/Reserved bit
*/

/* Foreground color definitions */
#define BLACK   0
#define GREEN   2
#define RED     4
#define YELLOW  14
#define WHITE   15

/* Global VGA state variables */
extern unsigned short *vga_buffer;
extern unsigned int   cursor_pos;

/* Core display functions */
int  vga_putchar(char c, unsigned char color);
int  vga_puts(const char *s, unsigned char color);
int  vga_newline(void);
int  vga_clear(void);

/* Cursor positioning functions */
void vga_set_cursor_pos(int x, int y);
void vga_puts_at(const char *s, unsigned char color, int x, int y);
void vga_puts_centered(const char *s, unsigned char color, int row);

/* String utility functions */
int  k_strcmp(const char *s1, const char *s2);
int  k_strlen(const char *s);

#endif /* SCREEN_H */
