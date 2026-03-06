#include "keyboard.h"
#include "screen.h"

#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64

#define PRESS_LEFT_SHIFT        0x2A
#define RELEASE_LEFT_SHIFT      0xAA
#define PRESS_RIGHT_SHIFT       0x36
#define RELEASE_RIGHT_SHIFT     0xB6
#define PRESS_CTRL              0x1D
#define RELEASE_CTRL            0x9D
#define RELEASE_MASK            0x80

static const char scancode_lowercase[] = {
    0,  27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y',
    'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';', '\'', '`', 0,
    '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0, '*', 0, ' ',
};

static const char scancode_uppercase[] = {
    0,  27, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y',
    'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H',
    'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?', 0, '*', 0, ' ',
};

static int new_key_event(void)
{
    return inb(KEYBOARD_STATUS_PORT) & 1;
}

static uint8_t get_scancode(void)
{
    return inb(KEYBOARD_DATA_PORT);
}

static char scancode_to_ascii(uint8_t scancode, int shift_pressed)
{
    if (scancode > sizeof(scancode_lowercase) / sizeof(char))
        return 0;
    return shift_pressed ? scancode_uppercase[scancode] : scancode_lowercase[scancode];
}

void handle_keyboard(void)
{
    int shift_pressed = 0;
    int ctrl_pressed = 0;
    
    while (1) {
        if (new_key_event()) {
            uint8_t scancode = get_scancode();
            
            if (scancode == 0xE0) {
                scancode = get_scancode();
                if (scancode == 0x48) {  // Arrow up
                    if (extra_scroll[screen_index] <= total_row[screen_index] - ROWS_COUNT) {
                        extra_scroll[screen_index]++;
                        display_screen(screen_index);
                    }
                } else if (scancode == 0x50) {  // Arrow down
                    if (extra_scroll[screen_index]) {
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
            } else if (scancode == 0x0E) {  // Backspace
                delete_char();
            } else if (!(scancode & RELEASE_MASK)) {
                char ascii = scancode_to_ascii(scancode, shift_pressed);
                if (ascii == '\n') {
                    exec_cmd();
                    new_prompt();
                } else if (ascii) {
                    print_char(ascii, WHITE);
                }
            }
        }
    }
}
