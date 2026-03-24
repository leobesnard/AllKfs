/* ************************************************************************** */
/*                                                                            */
/*   keyboard.c - Keyboard input handling                                     */
/*                                                                            */
/*   Implements keyboard polling, scancode translation, and special key       */
/*   handling including shift, ctrl, and arrow keys.                          */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include <screen.h>

/* ==========================================================================
   Keyboard Port Definitions
   ========================================================================== */

#define KB_DATA_PORT        0x60
#define KB_STATUS_PORT      0x64

/* ==========================================================================
   Modifier Key Scancodes
   ========================================================================== */

/* Left Shift Key */
#define SCANCODE_LSHIFT_PRESS       0x2A
#define SCANCODE_LSHIFT_RELEASE     0xAA

/* Right Shift Key */
#define SCANCODE_RSHIFT_PRESS       0x36
#define SCANCODE_RSHIFT_RELEASE     0xB6

/* Control Key */
#define SCANCODE_CTRL_PRESS         0x1D
#define SCANCODE_CTRL_RELEASE       0x9D

/* Key Release Bit Mask */
#define SCANCODE_RELEASE_MASK       0x80

/* ==========================================================================
   Scancode to ASCII Translation Tables
   ========================================================================== */

/* Lowercase/unshifted characters */
const char keymap_lowercase[] =
{
    0,  27, '1', '2', '3', '4', '5', '6',    /* 0x00 - 0x07 */
    '7', '8', '9', '0', '-', '=', '\b',      /* 0x08 - 0x0E (Backspace) */
    '\t', 'q', 'w', 'e', 'r', 't', 'y',      /* 0x0F - 0x15 (Tab) */
    'u', 'i', 'o', 'p', '[', ']', '\n',      /* 0x16 - 0x1C (Enter) */
    0,  'a', 's', 'd', 'f', 'g', 'h',        /* 0x1D - 0x23 (Ctrl) */
    'j', 'k', 'l', ';', '\'', '`', 0,        /* 0x24 - 0x2A (LShift) */
    '\\','z', 'x', 'c', 'v', 'b', 'n',       /* 0x2B - 0x31 */
    'm', ',', '.', '/', 0, '*', 0, ' ',      /* 0x32 - 0x39 (Space) */
};

/* Uppercase/shifted characters */
const char keymap_uppercase[] =
{
    0,  27, '!', '@', '#', '$', '%', '^',    /* 0x00 - 0x07 */
    '&', '*', '(', ')', '_', '+', '\b',      /* 0x08 - 0x0E */
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y',      /* 0x0F - 0x15 */
    'U', 'I', 'O', 'P', '{', '}', '\n',      /* 0x16 - 0x1C */
    0,  'A', 'S', 'D', 'F', 'G', 'H',        /* 0x1D - 0x23 */
    'J', 'K', 'L', ':', '"', '~', 0,         /* 0x24 - 0x2A */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',       /* 0x2B - 0x31 */
    'M', '<', '>', '?', 0, '*', 0, ' ',      /* 0x32 - 0x39 */
};

/* ==========================================================================
   Port I/O Helper
   ========================================================================== */

static inline unsigned char
inb(unsigned short port)
{
    unsigned char value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* ==========================================================================
   Keyboard Input Functions
   ========================================================================== */

int
keyboard_has_event(void)
{
    /* Check if keyboard data is available (status bit 0) */
    return inb(KB_STATUS_PORT) & 1;
}

unsigned char
keyboard_read_scancode(void)
{
    /* Read scancode from keyboard data port */
    return inb(KB_DATA_PORT);
}

char
keyboard_scancode_to_char(uint8_t scancode, int shift_active)
{
    /* Bounds check for our translation table */
    if (scancode > sizeof(keymap_lowercase) / sizeof(char))
    {
        return 0;
    }

    /* Return appropriate character based on shift state */
    return shift_active
        ? keymap_uppercase[scancode]
        : keymap_lowercase[scancode];
}

/* ==========================================================================
   Main Keyboard Handler
   ========================================================================== */

void
keyboard_handler(void)
{
    int shift_active = 0;
    int ctrl_active = 0;

    /* Main keyboard polling loop */
    while (1)
    {
        if (keyboard_has_event())
        {
            unsigned char scancode = keyboard_read_scancode();

            /* Handle extended scancodes (0xE0 prefix) */
            if (scancode == 0xE0)
            {
                scancode = keyboard_read_scancode();

                /* Arrow Up - Scroll up in buffer */
                if (scancode == 0x48)
                {
                    if (scroll_offset[active_screen] <= row_count[active_screen] - VGA_ROWS)
                    {
                        scroll_offset[active_screen]++;
                        screen_display(active_screen);
                    }
                }
                /* Arrow Down - Scroll down in buffer */
                else if (scancode == 0x50)
                {
                    if (scroll_offset[active_screen])
                    {
                        scroll_offset[active_screen]--;
                        screen_display(active_screen);
                    }
                }
            }
            /* Left Shift Press */
            else if (scancode == SCANCODE_LSHIFT_PRESS
                     || scancode == SCANCODE_RSHIFT_PRESS)
            {
                shift_active = 1;
            }
            /* Shift Release */
            else if (scancode == SCANCODE_LSHIFT_RELEASE
                     || scancode == SCANCODE_RSHIFT_RELEASE)
            {
                shift_active = 0;
            }
            /* Control Press */
            else if (scancode == SCANCODE_CTRL_PRESS)
            {
                ctrl_active = 1;
            }
            /* Control Release */
            else if (scancode == SCANCODE_CTRL_RELEASE)
            {
                ctrl_active = 0;
            }
            /* Ctrl+Key Combinations */
            else if (ctrl_active)
            {
                char ascii = keyboard_scancode_to_char(scancode, shift_active);

                /* Ctrl+1/2/3 - Switch between virtual screens */
                if (!(scancode & SCANCODE_RELEASE_MASK)
                    && ascii >= '1'
                    && ascii < ('1' + NUM_SCREENS))
                {
                    screen_switch(ascii - '1');
                }
            }
            /* Backspace Key */
            else if (scancode == 0x0E)
            {
                vga_delete_char();
            }
            /* Regular Key Press (ignore releases) */
            else if (!(scancode & SCANCODE_RELEASE_MASK))
            {
                char ascii = keyboard_scancode_to_char(scancode, shift_active);
                vga_putchar(ascii, WHITE);
            }
        }
    }

    return;
}
