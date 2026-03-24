/*
 * =============================================================================
 *                              KFS2 BONUS - SCREEN DRIVER
 * =============================================================================
 * VGA text mode display driver implementation
 * Handles character output, screen management, and shell commands
 * =============================================================================
 */

#include "screen.h"
#include "str.h"
#include "ft_printf.h"
#include "gdt.h"

/* =============================================================================
 *                              GLOBAL VARIABLES
 * ============================================================================= */

unsigned short* vga_buffer;
unsigned int cursor_pos = 0;
unsigned int row_count[NUM_SCREENS];
unsigned char last_scancode = 0;

/* =============================================================================
 *                              CHARACTER OUTPUT FUNCTIONS
 * ============================================================================= */

/*
 * vga_putchar - Print a single character to the screen
 * @c: Character to print
 * @color: VGA color attribute
 * Returns: 0 on success
 *
 * Handles printable characters and newlines
 * Manages buffer scrolling when reaching end of visible area
 */
int vga_putchar(char c, unsigned char color)
{
    scroll_offset[active_screen] = 0;

    if (c == '\n')
    {
        vga_newline();
        return 0;
    }
    else if (c >= 32 && c <= 126)
    {
        /* Write to visible VGA buffer if within screen bounds */
        if (cursor_pos < VGA_ROWS * VGA_COLS)
        {
            vga_buffer[cursor_pos] = c | (unsigned short)color << 8;
        }
        /* Always write to the screen stock buffer */
        stock[active_screen][cursor_pos] = c | (unsigned short)color << 8;
        cursor_pos++;
    }

    /* Check if we need to scroll or advance row count */
    if (cursor_pos % VGA_COLS == 0)
    {
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
    }

    /* Update display based on scroll state */
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

/*
 * vga_delete_char - Delete the character before cursor
 * Handles backspace functionality for shell input
 */
void vga_delete_char(void)
{
    if (cursor_pos == 0)
    {
        return;
    }

    /* Don't delete past the shell prompt marker (green space) */
    if (stock[active_screen][cursor_pos - 1] == (' ' | (unsigned short)GREEN << 8))
    {
        return;
    }

    cursor_pos--;

    /* Clear the character at current position */
    if (cursor_pos < VGA_ROWS * VGA_COLS)
    {
        vga_buffer[cursor_pos] = (' ' | (unsigned short)YELLOW << 8);
    }
    stock[active_screen][cursor_pos] = (' ' | (unsigned short)YELLOW << 8);

    /* Adjust row count if moving to previous line */
    if (cursor_pos % VGA_COLS == VGA_COLS - 1)
    {
        if (row_count[active_screen] == (BUFFER_ROWS - 1))
        {
            /* At maximum buffer, don't adjust */
        }
        else
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

/*
 * vga_puts - Print a null-terminated string
 * @s: String to print
 * @color: VGA color attribute
 */
void vga_puts(const char *s, unsigned char color)
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

/*
 * vga_puts_n - Print a string up to n characters
 * @s: String to print
 * @color: VGA color attribute
 * @n: Maximum number of characters to print
 */
void vga_puts_n(const char *s, unsigned char color, unsigned int n)
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

/*
 * vga_newline - Move cursor to the beginning of the next line
 * Handles line wrapping and buffer scrolling
 */
void vga_newline(void)
{
    /* Calculate offset to start of next line */
    int offset = VGA_COLS - ((cursor_pos) % VGA_COLS);

    /* Fill remaining characters on current line with spaces */
    for (; offset > 0; offset--)
    {
        stock[active_screen][cursor_pos] = ' ' | (unsigned short)WHITE << 8;
        cursor_pos++;
    }

    /* Handle scrolling if at buffer limit */
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

/*
 * vga_clear - Clear the entire screen
 * Returns: 0 on success
 */
int vga_clear(void)
{
    cursor_pos = 0;
    row_count[active_screen] = 0;

    /* Fill screen buffer with spaces */
    for (unsigned int i = 0; i < (BUFFER_ROWS * VGA_COLS); i++)
    {
        stock[active_screen][i] = ' ' | (unsigned int)YELLOW << 8;
    }

    cursor_pos = 0;
    row_count[active_screen] = 0;
    cursor_update();

    return 0;
}

/* =============================================================================
 *                              CURSOR MANAGEMENT FUNCTIONS
 * ============================================================================= */

/*
 * cursor_set_offset - Set cursor position by offset value
 * @offset: Linear position in VGA buffer (0 to VGA_ROWS*VGA_COLS-1)
 *
 * Programs the VGA hardware cursor position via I/O ports
 */
void cursor_set_offset(int offset)
{
    /* Send low byte of cursor position */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(offset & 0xFF));

    /* Send high byte of cursor position */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((offset >> 8)));
}

/*
 * cursor_update - Update hardware cursor to match current position
 */
void cursor_update(void)
{
    cursor_set_offset(cursor_pos);
}

/*
 * cursor_set_xy - Set cursor position by x,y coordinates
 * @x: Column position (0 to VGA_COLS-1)
 * @y: Row position (0 to VGA_ROWS-1)
 */
void cursor_set_xy(int x, int y)
{
    unsigned short position = y * VGA_COLS + x;
    cursor_set_offset(position);
}

/* =============================================================================
 *                              SHELL FUNCTIONS
 * ============================================================================= */

/*
 * shell_prompt - Display the shell command prompt
 * Format: kfs2@screenN>
 */
void shell_prompt(void)
{
    vga_newline();
    vga_puts("kfs2", WHITE);
    vga_puts("@", GREEN);
    vga_puts("screen", WHITE);
    kprintf("%d", active_screen + 1);
    vga_puts("> ", GREEN);
}

/*
 * shell_exec_cmd - Execute a shell command
 * Parses the current line for a command and executes it
 * Returns: Command result code, 0 if unknown command
 *
 * Supported commands:
 *   print-stack - Display kernel stack information
 *   gdt        - Display GDT entries
 *   halt       - Halt the CPU
 *   reboot     - Reboot the system
 *   shutdown   - Power off (QEMU/Bochs)
 */
int shell_exec_cmd(void)
{
    char cmdcpy[SHELL_CMD_BUFFER_SIZE + 1];
    int cmd_start_index = 1;

    /* Extract command from screen buffer (read backwards from cursor) */
    while (cmd_start_index <= SHELL_CMD_BUFFER_SIZE &&
           stock[active_screen][cursor_pos - cmd_start_index] != (' ' | (unsigned short)GREEN << 8))
    {
        cmdcpy[SHELL_CMD_BUFFER_SIZE - cmd_start_index] =
            *((char *)(&(stock[active_screen][cursor_pos - cmd_start_index])));
        cmd_start_index++;
    }
    cmdcpy[SHELL_CMD_BUFFER_SIZE] = 0;

    /* Command: print-stack */
    if (!k_strcmp(cmdcpy + SHELL_CMD_BUFFER_SIZE - cmd_start_index + 1, "print-stack"))
    {
        vga_newline();
        stack_display();
        return 1;
    }

    /* Command: gdt */
    if (!k_strcmp(cmdcpy + SHELL_CMD_BUFFER_SIZE - cmd_start_index + 1, "gdt"))
    {
        vga_newline();
        gdt_display();
        return 2;
    }

    /* Command: halt */
    if (!k_strcmp(cmdcpy + SHELL_CMD_BUFFER_SIZE - cmd_start_index + 1, "halt"))
    {
        system_halt();
        return 3;
    }

    /* Command: reboot */
    if (!k_strcmp(cmdcpy + SHELL_CMD_BUFFER_SIZE - cmd_start_index + 1, "reboot"))
    {
        system_reboot();
        return 4;
    }

    /* Command: shutdown */
    if (!k_strcmp(cmdcpy + SHELL_CMD_BUFFER_SIZE - cmd_start_index + 1, "shutdown"))
    {
        system_shutdown();
        return 5;
    }

    return 0;
}

/* =============================================================================
 *                              SYSTEM CONTROL FUNCTIONS
 * ============================================================================= */

/*
 * system_halt - Halt the CPU
 * Disables interrupts and stops execution
 */
void system_halt(void)
{
    __asm__ __volatile__("cli; hlt");
}

/*
 * system_reboot - Reboot the system
 * Uses the keyboard controller reset command
 */
void system_reboot(void)
{
    uint8_t status = 0x02;

    /* Wait for keyboard controller to be ready */
    while (status & 0x02)
    {
        status = inb(0x64);
    }

    /* Send reset command to keyboard controller */
    outb(0x64, 0xFE);

    /* Halt if reboot fails */
    system_halt();
}

/*
 * system_shutdown - Shutdown the system (QEMU/Bochs specific)
 * Sends ACPI power-off command
 */
void system_shutdown(void)
{
    outw(0x604, 0x2000);
}
