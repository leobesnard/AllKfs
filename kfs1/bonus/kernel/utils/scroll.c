/* ************************************************************************** */
/*                                                                            */
/*   scroll.c - Screen scrolling and buffer management                        */
/*                                                                            */
/*   Implements vertical scrolling with scrollback buffer support for         */
/*   multiple virtual screens.                                                */
/*                                                                            */
/* ************************************************************************** */

#include "screen.h"

/* ==========================================================================
   Arrow Key Scancodes (for reference)
   ========================================================================== */

#define KEY_ARROW_UP      0x48
#define KEY_ARROW_DOWN    0x50

/* ==========================================================================
   Global Screen Buffer Variables
   ========================================================================== */

/* Multi-screen buffer storage: rows * columns * 2 bytes per char */
unsigned short  stock[NUM_SCREENS][VGA_ROWS * VGA_COLS * 2];

/* Currently active screen index */
int             active_screen;

/* Stored cursor positions for each screen */
unsigned short  stock_cursor_index[NUM_SCREENS];

/* Scroll offset for scrollback functionality */
unsigned short  scroll_offset[NUM_SCREENS];

/* ==========================================================================
   Screen Scrolling Implementation
   ========================================================================== */

void
screen_scroll(void)
{
    /* Calculate how many rows have overflowed the visible area */
    unsigned int overflow_rows = row_count[active_screen] - VGA_ROWS + 1;

    /* Adjust for scroll offset (scrollback position) */
    if (overflow_rows > 0)
    {
        overflow_rows = scroll_offset[active_screen] > overflow_rows
            ? 0
            : overflow_rows - scroll_offset[active_screen];
    }

    /* Calculate starting position in buffer */
    unsigned int start = overflow_rows * VGA_COLS;
    int last_char_index = 0;

    /* Copy visible portion from buffer to VGA memory */
    for (unsigned int i = 0; i < (VGA_ROWS * VGA_COLS); i++)
    {
        /* Copy character or use space if empty */
        vga_buffer[i] = stock[active_screen][start + i]
            ? stock[active_screen][start + i]
            : ' ' | (unsigned short)WHITE << 8;

        /* Track position of last non-empty character */
        if (vga_buffer[i] && vga_buffer[i] != (' ' | (unsigned short)YELLOW << 8))
        {
            last_char_index = i;
        }
    }

    /* Position cursor after last character */
    cursor_set_offset(last_char_index + 1);
}
