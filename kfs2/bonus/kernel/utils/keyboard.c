/*
 * =============================================================================
 *                              KFS2 BONUS - KEYBOARD DRIVER
 * =============================================================================
 * PS/2 keyboard input driver implementation
 * Handles key presses, modifier keys, and screen navigation
 * =============================================================================
 */

#include "ft_printf.h"
#include "screen.h"

/* =============================================================================
 *                              KEYBOARD I/O PORTS
 * ============================================================================= */

#define KB_DATA_PORT        0x60    /* Keyboard data port */
#define KB_STATUS_PORT      0x64    /* Keyboard status port */

/* =============================================================================
 *                              MODIFIER KEY SCANCODES
 * ============================================================================= */

/* Left Shift key */
#define PRESS_LEFT_SHIFT        0x2A
#define RELEASE_LEFT_SHIFT      0xAA

/* Right Shift key */
#define PRESS_RIGHT_SHIFT       0x36
#define RELEASE_RIGHT_SHIFT     0xB6

/* Control key */
#define PRESS_CTRL              0x1D
#define RELEASE_CTRL            0x9D

/* Release bit mask (bit 7 set indicates key release) */
#define RELEASE_MASK            0x80

/* =============================================================================
 *                              SCANCODE TO ASCII TABLES
 * ============================================================================= */

/*
 * Lowercase scancode mapping (US QWERTY layout)
 * Index = scancode, value = ASCII character
 */
const char scancode_lowercase[] =
{
    0,   27,  '1', '2', '3', '4', '5', '6',     /* 0x00 - 0x07 */
    '7', '8', '9', '0', '-', '=', '\b',         /* 0x08 - 0x0E */
    '\t', 'q', 'w', 'e', 'r', 't', 'y',         /* 0x0F - 0x15 */
    'u', 'i', 'o', 'p', '[', ']', '\n',         /* 0x16 - 0x1C (Enter) */
    0,   'a', 's', 'd', 'f', 'g', 'h',          /* 0x1D - 0x23 */
    'j', 'k', 'l', ';', '\'', '`', 0,           /* 0x24 - 0x2A (Left shift) */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',         /* 0x2B - 0x31 */
    'm', ',', '.', '/', 0, '*', 0, ' ',         /* 0x32 - 0x39 (Space = 0x39) */
};

/*
 * Uppercase/shifted scancode mapping (US QWERTY layout)
 * Used when Shift is held down
 */
const char scancode_uppercase[] =
{
    0,   27,  '!', '@', '#', '$', '%', '^',     /* 0x00 - 0x07 */
    '&', '*', '(', ')', '_', '+', '\b',         /* 0x08 - 0x0E */
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y',         /* 0x0F - 0x15 */
    'U', 'I', 'O', 'P', '{', '}', '\n',         /* 0x16 - 0x1C */
    0,   'A', 'S', 'D', 'F', 'G', 'H',          /* 0x1D - 0x23 */
    'J', 'K', 'L', ':', '"', '~', 0,            /* 0x24 - 0x2A */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',          /* 0x2B - 0x31 */
    'M', '<', '>', '?', 0, '*', 0, ' ',         /* 0x32 - 0x39 */
};

/* =============================================================================
 *                              KEYBOARD INPUT FUNCTIONS
 * ============================================================================= */

/*
 * keyboard_has_event - Check if a keyboard event is available
 * Returns: Non-zero if a key event is pending
 */
int keyboard_has_event(void)
{
    return inb(KB_STATUS_PORT) & 1;
}

/*
 * keyboard_read_scancode - Read the current scancode from keyboard
 * Returns: The scancode of the key event
 */
unsigned char keyboard_read_scancode(void)
{
    return inb(KB_DATA_PORT);
}

/*
 * keyboard_scancode_to_char - Convert scancode to ASCII character
 * @sc: Scancode to convert
 * @shift_pressed: Non-zero if Shift key is held
 * Returns: ASCII character, or 0 if not printable
 */
char keyboard_scancode_to_char(uint8_t sc, int shift_pressed)
{
    if (sc > sizeof(scancode_lowercase) / sizeof(char))
    {
        return 0;   /* Out of bounds for this map */
    }
    return shift_pressed ? scancode_uppercase[sc] : scancode_lowercase[sc];
}

/* =============================================================================
 *                              MAIN KEYBOARD HANDLER
 * ============================================================================= */

/*
 * keyboard_handler - Main keyboard event processing loop
 *
 * This function runs the main input loop, processing:
 * - Regular key presses and releases
 * - Shift and Control modifier keys
 * - Arrow keys for scrolling
 * - Ctrl+1/2/3 for screen switching
 * - Enter key for command execution
 * - Backspace for character deletion
 */
void keyboard_handler(void)
{
    int shift_pressed = 0;
    int ctrl_pressed = 0;

    while (1)
    {
        if (keyboard_has_event())
        {
            unsigned char code = keyboard_read_scancode();

            /* Handle extended scancodes (arrow keys, etc.) */
            if (code == 0xE0)
            {
                code = keyboard_read_scancode();

                /* Up arrow - scroll up (view older content) */
                if (code == 0x48)
                {
                    if (scroll_offset[active_screen] <= row_count[active_screen] - VGA_ROWS)
                    {
                        scroll_offset[active_screen]++;
                        screen_display(active_screen);
                    }
                }
                /* Down arrow - scroll down (view newer content) */
                else if (code == 0x50)
                {
                    if (scroll_offset[active_screen])
                    {
                        scroll_offset[active_screen]--;
                        screen_display(active_screen);
                    }
                }
            }
            /* Left Shift pressed */
            else if (code == PRESS_LEFT_SHIFT || code == PRESS_RIGHT_SHIFT)
            {
                shift_pressed = 1;
            }
            /* Left Shift released */
            else if (code == RELEASE_LEFT_SHIFT || code == RELEASE_RIGHT_SHIFT)
            {
                shift_pressed = 0;
            }
            /* Control pressed */
            else if (code == PRESS_CTRL)
            {
                ctrl_pressed = 1;
            }
            /* Control released */
            else if (code == RELEASE_CTRL)
            {
                ctrl_pressed = 0;
            }
            /* Handle Ctrl+key combinations */
            else if (ctrl_pressed)
            {
                char ascii = keyboard_scancode_to_char(code, shift_pressed);

                /* Ctrl+1, Ctrl+2, Ctrl+3 - switch screens */
                if (!(code & RELEASE_MASK) && ascii >= '1' && ascii < ('1' + NUM_SCREENS))
                {
                    screen_switch(ascii - '1');
                }
            }
            /* Backspace key */
            else if (code == 0x0E)
            {
                vga_delete_char();
            }
            /* Regular key press (ignore releases) */
            else if (!(code & RELEASE_MASK))
            {
                char ascii = keyboard_scancode_to_char(code, shift_pressed);

                if (ascii == '\n')
                {
                    /* Execute shell command on Enter */
                    const int result = shell_exec_cmd();
                    if (result == 0)
                    {
                        /* Unknown command - could display error here */
                    }
                    shell_prompt();
                }
                else
                {
                    vga_putchar(ascii, WHITE);
                }
            }
        }
    }

    return;
}
