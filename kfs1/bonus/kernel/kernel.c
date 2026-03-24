/* ************************************************************************** */
/*                                                                            */
/*   kernel.c - Main kernel entry point                                       */
/*                                                                            */
/*   Initializes the VGA display, sets up multiple virtual screens,           */
/*   and enters the keyboard input loop.                                      */
/*                                                                            */
/* ************************************************************************** */

#include "screen.h"
#include "keyboard.h"
#include "ft_printf.h"

/* ==========================================================================
   Screen Initialization
   ========================================================================== */

void
init_screens(void)
{
    /* Map VGA buffer to video memory address */
    vga_buffer = (unsigned short *)VGA_MEMORY_BASE;

    /* Initialize each virtual screen */
    for (int i = 0; i < NUM_SCREENS; i++)
    {
        screen_switch(i);
        vga_clear();
        vga_puts("Welcome to screen", GREEN);
        kprintf(" %d ", i + 1);
        vga_puts("!\n", GREEN);
    }

    /* Switch back to first screen */
    screen_switch(0);
}

/* ==========================================================================
   Screen Switching
   ========================================================================== */

void
screen_switch(int new_screen_idx)
{
    /* Validate screen index */
    if (new_screen_idx < 0 || new_screen_idx >= NUM_SCREENS)
    {
        return;
    }

    /* Save current cursor position */
    stock_cursor_index[active_screen] = cursor_pos;

    /* Switch to new screen */
    active_screen = new_screen_idx;

    /* Restore cursor position for new screen */
    cursor_pos = stock_cursor_index[active_screen];

    /* Display the new screen */
    screen_display(active_screen);
}

/* ==========================================================================
   Screen Display
   ========================================================================== */

void
screen_display(int screen_idx)
{
    /* Validate screen index */
    if (screen_idx < 0 || screen_idx >= NUM_SCREENS)
    {
        return;
    }

    /* Handle scrolling if content exceeds visible rows */
    if (row_count[screen_idx] >= VGA_ROWS)
    {
        screen_scroll();
    }
    else
    {
        /* Copy buffer content to VGA memory */
        int last_char_index = 0;

        for (int u = 0; u < VGA_ROWS * VGA_COLS; u++)
        {
            vga_buffer[u] = stock[screen_idx][u];

            /* Track position of last non-empty character */
            if (vga_buffer[u]
                && vga_buffer[u] != (' ' | (unsigned short)YELLOW << 8))
            {
                last_char_index = u;
            }
        }

        /* Position cursor after last character */
        cursor_set_offset(last_char_index + 1);
    }
}

/* ==========================================================================
   Kernel Main Entry Point
   ========================================================================== */

void
main(void)
{
    /* Initialize cursor to top-left corner */
    cursor_set_offset(0);

    /* Initialize all virtual screens */
    init_screens();

    /* Enter keyboard input handler (infinite loop) */
    keyboard_handler();
}

/*
 * TODO:
 * - Refactor cursor detection logic
 * - Handle total_line++ buffer overflow
 * - Prevent overwriting page headers (welcome 1, 2, 3)
 */
