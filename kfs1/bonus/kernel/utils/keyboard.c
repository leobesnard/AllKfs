#include "ft_printf.h"
#include <screen.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Left Shift
#define PRESS_LEFT_SHIFT    0x2A
#define RELEASE_LEFT_SHIFT  0xAA

// Right Shift
#define PRESS_RIGHT_SHIFT   0x36
#define RELEASE_RIGHT_SHIFT 0xB6

// Ctrl
#define PRESS_CTRL     0x1D
#define RELEASE_CTRL   0x9D

#define RELEASE_MASK 0x80

const char scancode_lowercase[] = {
    0,  27, '1', '2', '3', '4', '5', '6',  // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=', '\b',   // 0x08 - 0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y',   // 0x0F - 0x15
    'u', 'i', 'o', 'p', '[', ']', '\n',   // 0x16 - 0x1C (Enter)
    0,  'a', 's', 'd', 'f', 'g', 'h',     // 0x1D - 0x23
    'j', 'k', 'l', ';', '\'', '`', 0,     // 0x24 - 0x2A (Left shift)
    '\\','z', 'x', 'c', 'v', 'b', 'n',    // 0x2B - 0x31
    'm', ',', '.', '/', 0, '*', 0, ' ',   // 0x32 - 0x39 (Space = 0x39)
    // Remaining scancodes can be extended as needed
};

const char scancode_uppercase[] = {
    0,  27, '!', '@', '#', '$', '%', '^',  // 0x00 - 0x07
    '&', '*', '(', ')', '_', '+', '\b',   // 0x08 - 0x0E
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y',   // 0x0F - 0x15
    'U', 'I', 'O', 'P', '{', '}', '\n',   // 0x16 - 0x1C
    0,  'A', 'S', 'D', 'F', 'G', 'H',     // 0x1D - 0x23
    'J', 'K', 'L', ':', '"', '~', 0,      // 0x24 - 0x2A
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',    // 0x2B - 0x31
    'M', '<', '>', '?', 0, '*', 0, ' ',   // 0x32 - 0x39
};

static inline unsigned char inb(unsigned short port) {
    // inb is a macro that reads a byte from the port
    unsigned char value;

    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

int new_key_event()
{
    return inb(KEYBOARD_STATUS_PORT) & 1;
}

unsigned char get_scancode()
{
    return inb(KEYBOARD_DATA_PORT);
}


char scancode_to_ascii(uint8_t scancode, int shift_pressed) {
    if (scancode > sizeof(scancode_lowercase) / sizeof(char)) return 0;  // Out of bounds for this map
    return shift_pressed ? scancode_uppercase[scancode] : scancode_lowercase[scancode];
}

void handle_keyboard()
{
    int shift_pressed = 0;
    int ctrl_pressed = 0;

    while (1)
    {
        if (new_key_event()) 
        {
            unsigned char scancode = get_scancode();

            if (scancode == 0xE0) {
                scancode = get_scancode();
                if (scancode == 0x48) {
                    if (extra_scroll[screen_index] <= total_row[screen_index] - ROWS_COUNT)
                    {
                        extra_scroll[screen_index]++;
                        display_screen(screen_index);
                    }
                } else if (scancode == 0x50) {
                    if (extra_scroll[screen_index])
                    {
                        extra_scroll[screen_index]--;
                        display_screen(screen_index);
                    }
                }
            } else if (scancode == PRESS_LEFT_SHIFT || scancode == PRESS_RIGHT_SHIFT) {
                shift_pressed = 1;
            } else if (scancode == RELEASE_LEFT_SHIFT || scancode == RELEASE_RIGHT_SHIFT) {
                shift_pressed = 0;
            } else if (scancode == PRESS_CTRL) {
                ctrl_pressed = 1;
            } else if (scancode == RELEASE_CTRL) {
                ctrl_pressed = 0;
            } else if (ctrl_pressed) {
                char ascii = scancode_to_ascii(scancode, shift_pressed);
                if (!(scancode & RELEASE_MASK) && ascii >= '1' && ascii < ('1' + SCREEN_COUNT)) {
                    switch_screen(ascii - '1');
                }
                //HANDLE SCREEN SWITCH;
            } else if (!(scancode & RELEASE_MASK)) { // Ignore key release events
                char ascii = scancode_to_ascii(scancode, shift_pressed);
                print_char(ascii, WHITE);
            }
        }
    }
    return ;
}