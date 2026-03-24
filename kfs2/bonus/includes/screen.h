/*
 * =============================================================================
 *                              KFS2 BONUS - SCREEN HEADER
 * =============================================================================
 * VGA text mode display driver header file
 * Provides functions for screen output, cursor management, and multi-screen support
 * =============================================================================
 */

#ifndef SCREEN_H
# define SCREEN_H

/* =============================================================================
 *                              VGA DISPLAY CONSTANTS
 * ============================================================================= */

# define VGA_ROWS           (25)
# define VGA_COLS           (80)
# define VGA_MEMORY_BASE    0xB8000

/* =============================================================================
 *                              MULTI-SCREEN CONSTANTS
 * ============================================================================= */

# define NUM_SCREENS        3
# define BUFFER_ROWS        (VGA_ROWS * 2)
# define SHELL_CMD_BUFFER_SIZE  60

/* =============================================================================
 *                              GLOBAL VARIABLES (EXTERN)
 * ============================================================================= */

/* VGA buffer pointer - points to video memory at 0xB8000 */
extern unsigned short* vga_buffer;

/* Screen buffer storage for multiple virtual screens */
extern unsigned short stock[NUM_SCREENS][BUFFER_ROWS * VGA_COLS];

/* Current active screen index */
extern int active_screen;

/* Cursor position for current screen */
extern unsigned int cursor_pos;

/* Stored cursor positions for each screen */
extern unsigned short stock_cursor_index[NUM_SCREENS];

/* Scroll offset for each screen (for scroll-back feature) */
extern unsigned short scroll_offset[NUM_SCREENS];

/* Total row count for each screen */
extern unsigned int row_count[NUM_SCREENS];

/* Last received keyboard scancode */
extern unsigned char last_scancode;

/* =============================================================================
 *                              VGA COLOR DEFINITIONS
 * ============================================================================= */

/*
 * VGA Color Byte Format:
 * Bits 0-3: Foreground color (16 colors)
 * Bits 4-6: Background color (8 colors)
 * Bit 7:    Blink attribute (often disabled)
 */

#define BLACK           0
#define BLUE            1
#define GREEN           2
#define CYAN            3
#define RED             4
#define MAGENTA         5
#define BROWN           6
#define LIGHT_GREY      7
#define DARK_GREY       8
#define LIGHT_BLUE      9
#define LIGHT_GREEN     10
#define LIGHT_CYAN      11
#define LIGHT_RED       12
#define LIGHT_MAGENTA   13
#define YELLOW          14
#define WHITE           15

/* =============================================================================
 *                              VGA OUTPUT FUNCTIONS
 * ============================================================================= */

int     vga_putchar(char c, unsigned char color);
void    vga_puts(const char *s, unsigned char color);
void    vga_puts_n(const char *s, unsigned char color, unsigned int n);
void    vga_newline(void);
int     vga_clear(void);
void    vga_delete_char(void);

/* =============================================================================
 *                              SCREEN MANAGEMENT FUNCTIONS
 * ============================================================================= */

void    screen_switch(int new_screen_index);
void    screen_scroll(void);
void    screen_display(int screen_idx);

/* =============================================================================
 *                              CURSOR MANAGEMENT FUNCTIONS
 * ============================================================================= */

void    cursor_update(void);
void    cursor_set_xy(int x, int y);
void    cursor_set_offset(int offset);

/* =============================================================================
 *                              SHELL FUNCTIONS
 * ============================================================================= */

void    shell_prompt(void);
int     shell_exec_cmd(void);

/* =============================================================================
 *                              SYSTEM CONTROL FUNCTIONS
 * ============================================================================= */

void    system_halt(void);
void    system_reboot(void);
void    system_shutdown(void);

/* =============================================================================
 *                              I/O PORT FUNCTIONS (INLINE)
 * ============================================================================= */

/*
 * outb - Write a byte to an I/O port
 * @port: The I/O port address
 * @value: The byte value to write
 */
static inline void outb(int port, int value)
{
    asm volatile ("outb %%al, %%dx" : : "a"(value), "d"(port));
}

/*
 * outw - Write a word (2 bytes) to an I/O port
 * @port: The I/O port address
 * @value: The word value to write
 */
static inline void outw(int port, int value)
{
    asm volatile ("outw %%ax, %%dx" : : "a"(value), "d"(port));
}

/*
 * inb - Read a byte from an I/O port
 * @port: The I/O port address
 * Returns: The byte read from the port
 */
static inline unsigned char inb(unsigned short port)
{
    unsigned char value;

    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#endif
