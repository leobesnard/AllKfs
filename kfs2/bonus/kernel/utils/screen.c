#include "screen.h"
#include "str.h"
#include "ft_printf.h"
#include "gdt.h"

unsigned short* screen_buffer;
unsigned int cursor_index = 0;
unsigned int total_row[SCREEN_COUNT];
unsigned char scancode = 0;

int print_char(char c, unsigned char color)
{
    extra_scroll[screen_index] = 0;
    if (c == '\n') {
        print_new_line();
        return 0;
    } else if (c >= 32 && c <= 126){
        if (cursor_index < ROWS_COUNT * COLUMNS_COUNT)
            screen_buffer[cursor_index] = c | (unsigned short)color << 8;
        stock[screen_index][cursor_index] = c | (unsigned short)color << 8;
        cursor_index++;
    }
    // le premier octet est le caractère lui-même (selon le code ASCII)
    // le second octet contient des informations sur la couleur de fond et la couleur du texte.
    // Bits 0-3 : Couleur du texte (ex: 0x07 = blanc).
    // Bits 4-7 : Couleur de fond (ex: 0x00 = fond noir).

    
    if (cursor_index % COLUMNS_COUNT == 0)
    {
        if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1))
        {
            for (int i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
            }
            for (int i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
            }
            cursor_index -= COLUMNS_COUNT;
        }
        else
        {
            total_row[screen_index]++;
        }
    }
    if (total_row[screen_index] >= ROWS_COUNT)
    {
        scroll_screen();
    }
    else
    {
        update_cursor();
    }
    
    return 0;
}

void delete_char() {
    if (cursor_index == 0)
        return ;
    if (stock[screen_index][cursor_index - 1] == (' ' | (unsigned short)GREEN<<8)) {
        return ;
    }
    // stock[screen_index][cursor_index] == (' ' | (unsigned short)YELLOW<<8);
    cursor_index--;

    // stock[screen_index][cursor_index] == (' ' | (unsigned short)YELLOW<<8);
    if (cursor_index < ROWS_COUNT * COLUMNS_COUNT)
        screen_buffer[cursor_index] = (' ' | (unsigned short)YELLOW<<8);
    stock[screen_index][cursor_index] = (' ' | (unsigned short)YELLOW<<8);

    if (cursor_index % COLUMNS_COUNT == COLUMNS_COUNT - 1)
    {
        if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1))
        {
            // for (int i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
            //     stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
            // }
            // for (int i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
            //     stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
            // }
            // cursor_index -= COLUMNS_COUNT;
        }
        else
        {
            total_row[screen_index]--;
        }
    }

    if (total_row[screen_index] >= ROWS_COUNT)
    {
        scroll_screen();
    }
    else
    {
        update_cursor();
    }
}

void print_str(const char *s, unsigned char color)
{
    unsigned int i = 0;

    while (s[i]) 
    {
        if (s[i] == '\n')
        {
            print_new_line();
        }
        else 
        {
            print_char(s[i], color);
        }
        i++;
    }
}

void print_str_n(const char *s, unsigned char color, unsigned int n)
{
    if (n < 0)
        return;
    unsigned int i = 0;

    while (s[i] && i < n) 
    {
        if (s[i] == '\n')
        {
            print_new_line();
        }
        else 
        {
            print_char(s[i], color);
        }
        i++;
    }
}

void print_new_line()
{
    // print_char('\n', WHITE);
    // return ;
    // if (cursor_index % COLUMNS_COUNT == 0)
    // {
    //     // + 80 = Ligne suivante si debut de ligne
    //     cursor_index += COLUMNS_COUNT;
    // }
    // else
    // {
    int offset = COLUMNS_COUNT - ((cursor_index) % COLUMNS_COUNT);
    for (; offset > 0; offset--) {
        stock[screen_index][cursor_index] = ' ' | (unsigned short)WHITE << 8;
        cursor_index++;
        // print_char(' ', WHITE);
    }
    if (total_row[screen_index] == (BUFFER_ROW_COUNT - 1))
        {
            for (int i = COLUMNS_COUNT; i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i - COLUMNS_COUNT] = stock[screen_index][i];
            }
            for (int i = COLUMNS_COUNT * (BUFFER_ROW_COUNT - 1); i < COLUMNS_COUNT * BUFFER_ROW_COUNT; i++) {
                stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
            }
            cursor_index -= COLUMNS_COUNT;
        }
        else
        {
            total_row[screen_index]++;
        }
    display_screen(screen_index);
    // }
    // total_row[screen_index]++;
    // if (total_row[screen_index] > ROWS_COUNT)
    // {
    //     scroll_screen();
    // }
    // else 
    // {
    //     update_cursor();
    // }
}

int clear_screen()
{
    cursor_index = 0;
    total_row[screen_index] = 0;
    for (unsigned int i = 0; i < (BUFFER_ROW_COUNT * COLUMNS_COUNT); i++)
    {
        stock[screen_index][i] = ' ' | (unsigned int)YELLOW << 8;
        // print_char(' ', YELLOW);
    }
    cursor_index = 0;
    total_row[screen_index] = 0;
    update_cursor();
    return 0;
}

void set_cursor_offset(int offset) 
{
    outb(0x3D4, 0x0F); // LOW BYTE
    outb(0x3D5, (uint8_t)(offset & 0xFF));

    outb(0x3D4, 0x0E); // HIGH BYTE
    outb(0x3D5, (uint8_t)((offset >> 8)));
}
void update_cursor() {
    set_cursor_offset(cursor_index);
}

void set_cursor(int x, int y) 
{
    unsigned short position = y * COLUMNS_COUNT + x;
    set_cursor_offset(position);
}

void new_prompt() {
    print_new_line();
    print_str("kfs2", WHITE);
    print_str("@", GREEN);
    print_str("screen", WHITE);
    kprintf("%d", screen_index + 1);
    print_str("> ", GREEN);
}

void halt(void) {
    __asm__ __volatile__("cli; hlt");
}

void reboot(void) {
    // Wait until the keyboard controller is ready
    // while ((inb(0x64) & 0x02) != 0);

    // Send reset command
    // outb(0xFE, 0x64);

    // Loop forever if reboot fails
    // while (1) {
    //     __asm__ __volatile__("hlt");
    // }
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    halt();
}

void shutdown(void)
{
    outw(0x604, 0x2000);
}

int exec_cmd()
{
    char cmdcpy[CMD_BUFFER_SIZE + 1];
    int cmd_start_index = 1;

    while (cmd_start_index <= CMD_BUFFER_SIZE && stock[screen_index][cursor_index - cmd_start_index] != (' ' | (unsigned short)GREEN<<8))
    {
        cmdcpy[CMD_BUFFER_SIZE - cmd_start_index] = *((char *)(&(stock[screen_index][cursor_index - cmd_start_index])));
        cmd_start_index++;
    }
    cmdcpy[CMD_BUFFER_SIZE] = 0;
    // print_str(cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1, RED);
    if (!kstrcmp(cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1, "print-stack"))
    {
        print_new_line();
        print_kernel_stack();
        return 1;

    }
    if (!kstrcmp(cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1, "gdt"))
    {
        print_new_line();
        print_gdt();
        return 2;

    }
    if (!kstrcmp(cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1, "halt"))
    {
        halt();
        return 3;

    }
    if (!kstrcmp(cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1, "reboot"))
    {
        reboot();
        return 4;
    }
    if (!kstrcmp(cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1, "shutdown"))
    {
        shutdown();
        return 5;
    }
    return 0;

}
