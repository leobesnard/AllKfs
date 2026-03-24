/*
 * =============================================================================
 *                              KFS2 BONUS - SCROLL DRIVER
 * =============================================================================
 * Screen scrolling and multi-screen buffer management
 * Handles virtual terminal scrolling with scroll-back support
 * =============================================================================
 */

#include "screen.h"

/* =============================================================================
 *                              ARROW KEY SCANCODES
 * ============================================================================= */

#define KEY_UP      0x48    /* Up arrow scancode */
#define KEY_DOWN    0x50    /* Down arrow scancode */

/* =============================================================================
 *                              GLOBAL VARIABLES
 * ============================================================================= */

/* Screen buffer storage: NUM_SCREENS virtual screens, each with scrollback */
unsigned short stock[NUM_SCREENS][VGA_ROWS * VGA_COLS * 2];

/* Index of currently active screen */
int active_screen;

/* Saved cursor positions for each screen */
unsigned short stock_cursor_index[NUM_SCREENS];

/* Scroll offset for each screen (lines scrolled back from bottom) */
unsigned short scroll_offset[NUM_SCREENS];

/* =============================================================================
 *                              SCROLL FUNCTIONS
 * ============================================================================= */

/*
 * screen_scroll - Scroll the current screen's visible area
 *
 * This function handles the display of scrollable content.
 * It calculates which portion of the screen buffer to display
 * based on the total rows and scroll offset.
 *
 * The scroll_offset allows viewing previous content (scroll-back feature).
 */
void screen_scroll(void)
{
    unsigned int overflow_rows = row_count[active_screen] - VGA_ROWS + 1;

    /* Adjust for scroll offset (user scrolling up to view history) */
    if (overflow_rows > 0)
    {
        overflow_rows = scroll_offset[active_screen] > overflow_rows
                        ? 0
                        : overflow_rows - scroll_offset[active_screen];
    }

    /* Calculate starting position in the buffer */
    unsigned int start = overflow_rows * VGA_COLS;
    int last_char_index = 0;

    /* Copy visible portion from stock to VGA buffer */
    for (unsigned int i = 0; i < (VGA_ROWS * VGA_COLS); i++)
    {
        vga_buffer[i] = stock[active_screen][start + i]
                        ? stock[active_screen][start + i]
                        : ' ' | (unsigned short)WHITE << 8;

        /* Track the last non-empty character position */
        if (vga_buffer[i] && vga_buffer[i] != (' ' | (unsigned short)YELLOW << 8))
        {
            last_char_index = i;
        }
    }

    /* Position cursor after the last character */
    cursor_set_offset(last_char_index + 1);
}
