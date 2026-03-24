/* ************************************************************************** */
/*                                                                            */
/*   screen.h - VGA text mode interface                                       */
/*                                                                            */
/*   This header defines the VGA text mode interface for the kernel.          */
/*   It provides functions for text output, cursor control, and multiple      */
/*   virtual screen support.                                                  */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCREEN_H
# define SCREEN_H

/* ==========================================================================
   VGA Display Constants
   ========================================================================== */

# define VGA_ROWS           (25)
# define VGA_COLS           (80)
# define VGA_MEMORY_BASE    0xB8000

/* ==========================================================================
   Multi-Screen Buffer Configuration
   ========================================================================== */

# define NUM_SCREENS        3
# define BUFFER_ROWS        (VGA_ROWS * 2)

/* ==========================================================================
   VGA Color Definitions
   ========================================================================== */

/*
 * VGA attribute byte format:
 *
 *   Bit:  7   6   5   4   3   2   1   0
 *         |   |   |   |   |   |   |   |
 *         |   +---+---+   +---+---+---+
 *         |       |           |
 *         |       |           +-- Foreground color (4 bits)
 *         |       +-------------- Background color (3 bits)
 *         +---------------------- Blink/Intensity (1 bit)
 */

# define BLACK      0
# define GREEN      2
# define RED        4
# define YELLOW     14
# define WHITE      15

/* ==========================================================================
   Global Variables
   ========================================================================== */

/* Pointer to VGA video memory */
extern unsigned short       *vga_buffer;

/* Screen buffer storage for multiple virtual screens */
extern unsigned short       stock[NUM_SCREENS][BUFFER_ROWS * VGA_COLS];

/* Current active screen index */
extern int                  active_screen;

/* Cursor positions for each screen */
extern unsigned short       stock_cursor_index[NUM_SCREENS];

/* Scroll offset for each screen (for scrollback) */
extern unsigned short       scroll_offset[NUM_SCREENS];

/* Current cursor position in the buffer */
extern unsigned int         cursor_pos;

/* Total row count for each screen */
extern unsigned int         row_count[NUM_SCREENS];

/* Current keyboard scancode */
extern unsigned char        scancode;

/* ==========================================================================
   VGA Output Functions
   ========================================================================== */

int     vga_putchar(char c, unsigned char color);
void    vga_puts(const char *s, unsigned char color);
void    vga_puts_n(const char *s, unsigned char color, unsigned int n);
void    vga_newline(void);
int     vga_clear(void);
void    vga_delete_char(void);

/* ==========================================================================
   Screen Management Functions
   ========================================================================== */

void    screen_switch(int new_screen_idx);
void    screen_display(int screen_idx);
void    screen_scroll(void);

/* ==========================================================================
   Cursor Control Functions
   ========================================================================== */

void    cursor_update(void);
void    cursor_set_xy(int x, int y);
void    cursor_set_offset(int offset);

/* ==========================================================================
   String Utility Functions
   ========================================================================== */

int     kstrlen(char *s);

#endif
