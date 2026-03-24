/* ************************************************************************** */
/*                                                                            */
/*   screen.c - VGA text mode display driver implementation                   */
/*                                                                            */
/* ************************************************************************** */

#include "screen.h"

/* Global VGA state */
unsigned short *vga_buffer;
unsigned int   cursor_pos = 0;

/*
** Internal helper: calculate string length
** Returns the number of characters in the string
*/
static int
get_string_length(const char *s)
{
    int len = 0;

    while (s[len])
    {
        len++;
    }
    return len;
}

/*
** Write a single character to VGA buffer at cursor position
** The character is combined with color attribute into a 16-bit value:
** - Lower byte: ASCII character code
** - Upper byte: color attribute (foreground + background)
*/
int
vga_putchar(char c, unsigned char color)
{
    vga_buffer[cursor_pos] = c | (unsigned short)color << 8;
    cursor_pos++;
    return 0;
}

/*
** Write a null-terminated string to VGA buffer
** Returns the number of characters written
*/
int
vga_puts(const char *s, unsigned char color)
{
    int idx = 0;

    while (s[idx])
    {
        vga_putchar(s[idx], color);
        idx++;
    }
    return idx;
}

/*
** Move cursor to the beginning of the next line
** Calculates remaining columns and advances cursor accordingly
*/
int
vga_newline(void)
{
    cursor_pos += VGA_COLS - ((cursor_pos) % VGA_COLS);
    return 0;
}

/*
** Clear the entire screen by zeroing all VGA buffer entries
** Also resets cursor position to top-left corner
*/
int
vga_clear(void)
{
    unsigned int i;

    for (i = 0; i < (VGA_ROWS * VGA_COLS); i++)
    {
        vga_buffer[i] = 0;
    }
    cursor_pos = 0;
    return 0;
}

/*
** Set cursor to specific column (x) and row (y) position
** Coordinates are clamped to valid screen bounds
*/
void
vga_set_cursor_pos(int x, int y)
{
    if (x < 0)
        x = 0;
    if (x >= VGA_COLS)
        x = VGA_COLS - 1;
    if (y < 0)
        y = 0;
    if (y >= VGA_ROWS)
        y = VGA_ROWS - 1;
    cursor_pos = (y * VGA_COLS) + x;
}

/*
** Write string at specified screen position
*/
void
vga_puts_at(const char *s, unsigned char color, int x, int y)
{
    vga_set_cursor_pos(x, y);
    vga_puts(s, color);
}

/*
** Write string centered horizontally on specified row
** Calculates the starting column to center the text
*/
void
vga_puts_centered(const char *s, unsigned char color, int row)
{
    int len;
    int x;

    len = get_string_length(s);
    x = (VGA_COLS - len) / 2;
    if (x < 0)
        x = 0;
    vga_set_cursor_pos(x, row);
    vga_puts(s, color);
}
