/**
 * =============================================================================
 * screen.h - VGA Text Mode Display Driver Header
 * =============================================================================
 * 
 * This header defines the interface for VGA text mode (mode 3) display
 * operations. VGA text mode provides an 80x25 character display with
 * 16 foreground and 16 background colors.
 * 
 * Memory Layout:
 *   - Each character cell occupies 2 bytes
 *   - Byte 0: ASCII character code
 *   - Byte 1: Attribute byte (foreground + background colors)
 * 
 * =============================================================================
 */

#ifndef SCREEN_H
# define SCREEN_H

/* =============================================================================
 * VGA Display Dimensions
 * ============================================================================= */

# define VGA_ROWS           (25)
# define VGA_COLS           (80)

/* =============================================================================
 * VGA Memory Configuration
 * ============================================================================= */

# define VGA_MEMORY_BASE    0xB8000

/* =============================================================================
 * Global State Variables
 * ============================================================================= */

extern unsigned short   *vga_buffer;    /* Pointer to VGA memory buffer */
extern unsigned int     cursor_pos;     /* Current cursor position index */

/* =============================================================================
 * VGA Color Palette Definitions
 * 
 * Standard 16-color VGA palette used for text mode display.
 * Colors 0-7 are normal intensity, colors 8-15 are high intensity.
 * ============================================================================= */

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
 * Function Prototypes
 * ============================================================================= */

/**
 * Write a single character to the VGA buffer at the current cursor position.
 * 
 * @param c     The ASCII character to display
 * @param color The color attribute (foreground | background << 4)
 * @return      Always returns 0
 */
int vga_putchar(char c, unsigned char color);

/**
 * Write a null-terminated string to the VGA buffer.
 * 
 * @param s     Pointer to the string to display
 * @param color The color attribute for all characters
 * @return      Number of characters written
 */
int vga_puts(const char *s, unsigned char color);

/**
 * Advance the cursor to the beginning of the next line.
 * 
 * @return Always returns 0
 */
int vga_newline(void);

/**
 * Clear the entire screen and reset cursor to position 0.
 * 
 * @return Always returns 0
 */
int vga_clear(void);

#endif /* SCREEN_H */
