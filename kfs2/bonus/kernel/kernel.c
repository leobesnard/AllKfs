/*
 * =============================================================================
 *                              KFS2 BONUS - KERNEL MAIN
 * =============================================================================
 * Main kernel entry point and initialization
 * Sets up the display, GDT, and starts the shell
 * =============================================================================
 */

#include "screen.h"
#include "ft_printf.h"
#include "keyboard.h"
#include "gdt.h"

/* =============================================================================
 *                              INITIALIZATION FUNCTIONS
 * ============================================================================= */

/*
 * init_screens - Initialize all virtual screens
 *
 * Sets up the VGA buffer pointer, clears all screens,
 * displays welcome messages, initializes GDT,
 * and shows shell prompts on each screen.
 */
void init_screens(void)
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

    /* Switch to first screen and set up GDT */
    screen_switch(0);
    vga_newline();
    gdt_init();

    /* Display shell prompt on all screens */
    for (int i = 0; i < NUM_SCREENS; i++)
    {
        screen_switch(i);
        shell_prompt();
    }

    /* Return to first screen */
    screen_switch(0);
}

/* =============================================================================
 *                              SCREEN MANAGEMENT FUNCTIONS
 * ============================================================================= */

/*
 * screen_switch - Switch to a different virtual screen
 * @new_screen_index: Index of screen to switch to (0 to NUM_SCREENS-1)
 *
 * Saves current cursor position, loads new screen's cursor,
 * and refreshes the display.
 */
void screen_switch(int new_screen_index)
{
    if (new_screen_index < 0 || new_screen_index >= NUM_SCREENS)
    {
        return;
    }

    /* Save current screen's cursor position */
    stock_cursor_index[active_screen] = cursor_pos;

    /* Switch to new screen */
    active_screen = new_screen_index;

    /* Restore new screen's cursor position */
    cursor_pos = stock_cursor_index[active_screen];

    /* Refresh display */
    screen_display(active_screen);
}

/*
 * screen_display - Refresh the display for a given screen
 * @screen_idx: Index of screen to display
 *
 * Copies screen buffer to VGA memory and updates cursor.
 * Handles scrolling if content exceeds visible area.
 */
void screen_display(int screen_idx)
{
    if (screen_idx < 0 || screen_idx >= NUM_SCREENS)
    {
        return;
    }

    /* If content exceeds visible rows, use scroll function */
    if (row_count[screen_idx] >= VGA_ROWS)
    {
        screen_scroll();
    }
    else
    {
        int last_char_index = 0;

        /* Copy screen buffer to VGA memory */
        for (int u = 0; u < VGA_ROWS * VGA_COLS; u++)
        {
            vga_buffer[u] = stock[screen_idx][u];

            /* Track last non-empty character */
            if (vga_buffer[u] && vga_buffer[u] != (' ' | (unsigned short)YELLOW << 8))
            {
                last_char_index = u;
            }
        }

        /* Position cursor after last character */
        cursor_set_offset(last_char_index + 1);
    }
}

/* =============================================================================
 *                              MAIN ENTRY POINT
 * ============================================================================= */

/*
 * main - Kernel entry point
 *
 * Called by boot.asm after setting up the stack.
 * Initializes the display and enters the keyboard handler loop.
 */
void main(void)
{
    /* Reset cursor to top-left */
    cursor_set_offset(0);

    /* Initialize all screens and GDT */
    init_screens();

    /* Enter main input loop (never returns) */
    keyboard_handler();
}
