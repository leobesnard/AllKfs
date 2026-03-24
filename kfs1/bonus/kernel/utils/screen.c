/* ************************************************************************** */
/*                                                                            */
/*   screen.c - VGA text mode output implementation                           */
/*                                                                            */
/*   Implements character output, string printing, cursor control, and        */
/*   screen clearing for VGA text mode display.                               */
/*                                                                            */
/* ************************************************************************** */

#include "screen.h"
#include "ft_printf.h"

/* ==========================================================================
   Global Variables
   ========================================================================== */

unsigned short  *vga_buffer;
unsigned int    cursor_pos = 0;
unsigned int    row_count[NUM_SCREENS];
unsigned char   scancode = 0;

/* ==========================================================================
   Port I/O Helper
   ========================================================================== */

static inline void
outb(int port, int value)
{
    /* Write a byte to the specified I/O port */
    asm volatile ("outb %%al, %%dx" : : "a"(value), "d"(port));
}

/* ==========================================================================
   Cursor Control Functions
   ========================================================================== */

void
cursor_set_offset(int offset)
{
    /* Send low byte of cursor position */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(offset & 0xFF));

    /* Send high byte of cursor position */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((offset >> 8)));
}

void
cursor_update(void)
{
    cursor_set_offset(cursor_pos);
}

void
cursor_set_xy(int x, int y)
{
    unsigned short position = y * VGA_COLS + x;
    cursor_set_offset(position);
}

/* ==========================================================================
   Character Output Functions
   ========================================================================== */

int
vga_putchar(char c, unsigned char color)
{
    /* Reset scroll offset when typing */
    scroll_offset[active_screen] = 0;

    /* Handle newline character */
    if (c == '\n')
    {
        vga_newline();
        return 0;
    }

    /* Handle printable characters (ASCII 32-126) */
    if (c >= 32 && c <= 126)
    {
        /* Write to VGA memory if within visible area */
        if (cursor_pos < VGA_ROWS * VGA_COLS)
        {
            vga_buffer[cursor_pos] = c | (unsigned short)color << 8;
        }

        /* Always write to the screen buffer */
        stock[active_screen][cursor_pos] = c | (unsigned short)color << 8;
        cursor_pos++;
    }

    /*
     * VGA character format:
     * - First byte: ASCII character code
     * - Second byte: Attribute byte (foreground + background color)
     *   Bits 0-3: Foreground color
     *   Bits 4-7: Background color
     */

    /* Check if we've reached the end of a row */
    if (cursor_pos % VGA_COLS == 0)
    {
        /* Check if we've reached the buffer limit */
        if (row_count[active_screen] == (BUFFER_ROWS - 1))
        {
            /* Scroll the buffer up by one line */
            for (int i = VGA_COLS; i < VGA_COLS * BUFFER_ROWS; i++)
            {
                stock[active_screen][i - VGA_COLS] = stock[active_screen][i];
            }

            /* Clear the last line */
            for (int i = VGA_COLS * (BUFFER_ROWS - 1); i < VGA_COLS * BUFFER_ROWS; i++)
            {
                stock[active_screen][i] = ' ' | (unsigned int)YELLOW << 8;
            }

            cursor_pos -= VGA_COLS;
        }
        else
        {
            row_count[active_screen]++;
        }
    }

    /* Update display based on row count */
    if (row_count[active_screen] >= VGA_ROWS)
    {
        screen_scroll();
    }
    else
    {
        cursor_update();
    }

    return 0;
}

void
vga_delete_char(void)
{
    /* Prevent deletion at buffer start */
    if (cursor_pos == 0)
    {
        return;
    }

    /* Move cursor back one position */
    cursor_pos--;

    /* Clear character in VGA memory if visible */
    if (cursor_pos < VGA_ROWS * VGA_COLS)
    {
        vga_buffer[cursor_pos] = ' ' | (unsigned short)YELLOW << 8;
    }

    /* Clear character in screen buffer */
    stock[active_screen][cursor_pos] = ' ' | (unsigned short)YELLOW << 8;

    /* Handle moving back to previous row */
    if (cursor_pos % VGA_COLS == VGA_COLS - 1)
    {
        if (row_count[active_screen] > 0)
        {
            row_count[active_screen]--;
        }
    }

    /* Update display */
    if (row_count[active_screen] >= VGA_ROWS)
    {
        screen_scroll();
    }
    else
    {
        cursor_update();
    }
}

/* ==========================================================================
   String Output Functions
   ========================================================================== */

void
vga_puts(const char *s, unsigned char color)
{
    unsigned int i = 0;

    while (s[i])
    {
        if (s[i] == '\n')
        {
            vga_newline();
        }
        else
        {
            vga_putchar(s[i], color);
        }
        i++;
    }
}

void
vga_puts_n(const char *s, unsigned char color, unsigned int n)
{
    if (n < 0)
    {
        return;
    }

    unsigned int i = 0;

    while (s[i] && i < n)
    {
        if (s[i] == '\n')
        {
            vga_newline();
        }
        else
        {
            vga_putchar(s[i], color);
        }
        i++;
    }
}

/* ==========================================================================
   Line Management Functions
   ========================================================================== */

void
vga_newline(void)
{
    /* Calculate offset to next line start */
    int offset = VGA_COLS - ((cursor_pos) % VGA_COLS);

    /* Fill remaining space on current line */
    for (; offset > 0; offset--)
    {
        stock[active_screen][cursor_pos] = ' ' | (unsigned short)WHITE << 8;
        cursor_pos++;
    }

    /* Check if buffer is full */
    if (row_count[active_screen] == (BUFFER_ROWS - 1))
    {
        /* Scroll buffer up by one line */
        for (int i = VGA_COLS; i < VGA_COLS * BUFFER_ROWS; i++)
        {
            stock[active_screen][i - VGA_COLS] = stock[active_screen][i];
        }

        /* Clear the last line */
        for (int i = VGA_COLS * (BUFFER_ROWS - 1); i < VGA_COLS * BUFFER_ROWS; i++)
        {
            stock[active_screen][i] = ' ' | (unsigned int)YELLOW << 8;
        }

        cursor_pos -= VGA_COLS;
    }
    else
    {
        row_count[active_screen]++;
    }

    screen_display(active_screen);
}

/* ==========================================================================
   Screen Control Functions
   ========================================================================== */

int
vga_clear(void)
{
    cursor_pos = 0;
    row_count[active_screen] = 0;

    /* Clear entire buffer with yellow attribute */
    for (unsigned int i = 0; i < (BUFFER_ROWS * VGA_COLS); i++)
    {
        stock[active_screen][i] = ' ' | (unsigned int)YELLOW << 8;
    }

    cursor_pos = 0;
    row_count[active_screen] = 0;
    cursor_update();

    return 0;
}

/* ==========================================================================
   String Utility Functions
   ========================================================================== */

int
kstrlen(char *s)
{
    char *new = s;

    while (*new)
    {
        new++;
    }

    return new - s;
}
