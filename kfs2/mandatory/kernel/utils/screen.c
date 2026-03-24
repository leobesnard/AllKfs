/**
 * =============================================================================
 * screen.c - VGA Text Mode Display Driver Implementation
 * =============================================================================
 * 
 * Implementation of VGA text mode operations for 80x25 character display.
 * Handles character output, string printing, cursor management, and
 * screen clearing operations.
 * 
 * =============================================================================
 */

#include "screen.h"

/* =============================================================================
 * Global State Variables
 * ============================================================================= */

unsigned short  *vga_buffer;
unsigned int    cursor_pos = 0;

/* =============================================================================
 * Helper Functions
 * ============================================================================= */

/**
 * vga_newline - Move cursor to the start of the next line
 * 
 * Calculates the offset needed to reach the next line boundary
 * and advances the cursor position accordingly.
 * 
 * @return Always returns 0
 */
int vga_newline(void)
{
    cursor_pos += VGA_COLS - ((cursor_pos) % VGA_COLS);
    return 0;
}

/**
 * vga_clear - Clear the entire VGA display buffer
 * 
 * Writes null characters to every cell in the VGA buffer,
 * effectively clearing the screen. Also resets the cursor
 * position to the top-left corner.
 * 
 * @return Always returns 0
 */
int vga_clear(void)
{
    unsigned int i;

    for (i = 0; i < (VGA_ROWS * VGA_COLS); i++)
    {
        vga_buffer[i] = 0;
    }
    cursor_pos = 0;
    return 0;
}

/* =============================================================================
 * Primary Display Functions
 * ============================================================================= */

/**
 * vga_putchar - Write a single character to the VGA buffer
 * 
 * Writes the character and its color attribute to the current
 * cursor position. The VGA buffer stores each cell as a 16-bit
 * value where the lower byte is the ASCII code and the upper
 * byte is the color attribute.
 * 
 * Layout: [color:8][ascii:8]
 *   - Bits 0-3: Foreground color (text)
 *   - Bits 4-7: Background color
 * 
 * @param c     ASCII character to display
 * @param color Color attribute byte
 * @return      Always returns 0
 */
int vga_putchar(char c, unsigned char color)
{
    vga_buffer[cursor_pos] = c | (unsigned short)color << 8;
    cursor_pos++;

    return 0;
}

/**
 * vga_puts - Write a null-terminated string to the VGA buffer
 * 
 * Iterates through each character in the string and calls
 * vga_putchar to display it at the current cursor position.
 * 
 * @param s     Pointer to null-terminated string
 * @param color Color attribute for all characters
 * @return      Number of characters written
 */
int vga_puts(const char *s, unsigned char color)
{
    int index;

    index = 0;
    while (s[index])
    {
        vga_putchar(s[index], color);
        index++;
    }

    return index;
}
