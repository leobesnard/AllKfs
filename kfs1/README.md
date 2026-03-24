# KFS-1: GRUB, Boot and Screen

## 📚 Table of Contents

1. [Project Overview](#project-overview)
2. [Architecture](#architecture)
3. [Core Concepts](#core-concepts)
   - [The Boot Process](#the-boot-process)
   - [Multiboot Specification](#multiboot-specification)
   - [VGA Text Mode](#vga-text-mode)
4. [Mandatory Part Implementation](#mandatory-part-implementation)
   - [Boot Assembly](#boot-assembly-bootasm)
   - [Linker Script](#linker-script-linkerld)
   - [Screen Driver](#screen-driver)
   - [Kernel Entry Point](#kernel-entry-point)
   - [Build System](#build-system-makefile)
5. [Bonus Part Implementation](#bonus-part-implementation)
   - [Multiple Screens](#multiple-virtual-screens)
   - [Keyboard Input](#keyboard-input-handling)
   - [Scrolling](#scroll-support)
   - [Printf Implementation](#printf-implementation)
6. [Technical Deep Dive](#technical-deep-dive)
7. [Building and Running](#building-and-running)
8. [References](#references)

---

## Project Overview

KFS-1 (Kernel From Scratch - Part 1) is the first project in a series aimed at building a complete operating system kernel from the ground up. This project focuses on the fundamental requirements to get a kernel booting and displaying output to the screen.

### Objectives Achieved

- ✅ Bootable kernel via GRUB bootloader
- ✅ ASM boot code handling Multiboot specification
- ✅ Basic kernel library with helper functions
- ✅ VGA text mode screen interface
- ✅ Display "42" on screen (mandatory requirement)
- ✅ [Bonus] Multiple virtual screens with switching
- ✅ [Bonus] Keyboard input handling
- ✅ [Bonus] Scroll support with buffer
- ✅ [Bonus] Printf implementation for formatted output
- ✅ [Bonus] Color support in text output

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         GRUB Bootloader                         │
│    Loads kernel at 1MB, sets up 32-bit protected mode           │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    boot.asm (Assembly Entry)                     │
│    - Multiboot header                                           │
│    - Stack setup                                                │
│    - Jump to C main()                                           │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    kernel.c (Main Kernel)                        │
│    - Initialize screen                                          │
│    - Display output                                             │
│    - [Bonus] Handle keyboard events                             │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Hardware Interface Layer                      │
│    screen.c    │   keyboard.c   │   scroll.c   │   printf       │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      VGA Memory (0xB8000)                        │
│          Physical memory-mapped I/O for text display             │
└─────────────────────────────────────────────────────────────────┘
```

### File Structure

```
kfs1/
├── mandatory/                    # Core implementation
│   ├── asm/
│   │   └── boot.asm             # Assembly boot code
│   ├── includes/
│   │   └── screen.h             # Screen driver header
│   ├── kernel/
│   │   ├── kernel.c             # Main kernel entry
│   │   └── utils/
│   │       ├── screen.c         # VGA text mode driver
│   │       └── str.c            # String utilities
│   ├── iso/
│   │   └── boot/grub/
│   │       └── grub.cfg         # GRUB configuration
│   ├── linker.ld                # Custom linker script
│   └── Makefile                 # Build configuration
│
└── bonus/                        # Extended features
    ├── asm/
    │   └── boot.asm             # Boot code (larger stack)
    ├── includes/
    │   ├── screen.h             # Extended screen header
    │   └── keyboard.h           # Keyboard driver header
    ├── kernel/
    │   ├── kernel.c             # Enhanced kernel
    │   └── utils/
    │       ├── screen.c         # Extended screen driver
    │       ├── scroll.c         # Scroll implementation
    │       ├── keyboard.c       # Keyboard handler
    │       └── str.c            # String utilities
    ├── printf/                   # Printf implementation
    │   ├── include/
    │   │   └── ft_printf.h      # Printf header with va_args
    │   └── srcs/
    │       ├── ft_printf.c      # Main printf function
    │       ├── ft_choice.c      # Format specifier handlers
    │       ├── other_fonctions.c
    │       ├── other_fonctions_2.c
    │       └── other_fonction_3.c
    ├── iso/
    │   └── boot/grub/
    │       └── grub.cfg         # GRUB configuration
    ├── linker.ld                # Custom linker script
    └── Makefile                 # Build configuration
```

---

## Core Concepts

### The Boot Process

When a computer starts, it follows a specific sequence:

1. **BIOS/UEFI Initialization**: Hardware POST (Power-On Self-Test)
2. **Bootloader Loading**: BIOS loads GRUB from the boot sector
3. **Kernel Loading**: GRUB reads the kernel from the ISO filesystem
4. **Protected Mode**: GRUB switches CPU to 32-bit protected mode
5. **Kernel Execution**: Control transfers to our kernel's entry point

```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│   BIOS   │───▶│   GRUB   │───▶│  boot.asm│───▶│ kernel.c │
│   POST   │    │Bootloader│    │  Entry   │    │   Main   │
└──────────┘    └──────────┘    └──────────┘    └──────────┘
```

### Multiboot Specification

The Multiboot specification is a standard that allows bootloaders (like GRUB) to load kernels in a uniform way. Our kernel must include a **Multiboot Header** at the beginning of the binary.

#### Multiboot Header Structure

```
Offset  │ Type   │ Field Name  │ Value        │ Purpose
────────┼────────┼─────────────┼──────────────┼─────────────────────────────
0       │ u32    │ magic       │ 0x1BADB002   │ Identifies multiboot kernel
4       │ u32    │ flags       │ 0x0          │ Feature flags (none needed)
8       │ u32    │ checksum    │ -(magic+flags)│ Must sum to 0 with magic+flags
```

**Why 0x1BADB002?**
This is the "magic number" that GRUB looks for. The letters spell "BADB00T" (bad boot) in hexadecimal - a playful identifier chosen by the specification authors.

**Checksum Calculation:**
```
magic + flags + checksum = 0
0x1BADB002 + 0x0 + checksum = 0
checksum = -(0x1BADB002 + 0x0)
```

### VGA Text Mode

VGA (Video Graphics Array) text mode is the simplest way to display text on screen. The video memory is mapped to physical address **0xB8000**.

#### Memory Layout

```
Address 0xB8000
    │
    ▼
┌────┬────┬────┬────┬────┬────┬────┬────┬─────┐
│Chr0│Att0│Chr1│Att1│Chr2│Att2│Chr3│Att3│ ... │
└────┴────┴────┴────┴────┴────┴────┴────┴─────┘
  │    │
  │    └── Attribute byte (colors)
  └─────── Character byte (ASCII)
```

#### Screen Dimensions
- **80 columns** × **25 rows** = 2000 characters
- Each character = 2 bytes (character + attribute)
- Total video memory used = **4000 bytes**

#### Attribute Byte Format

```
Bit:  7   6   5   4   3   2   1   0
      │   └───┬───┘   └─────┬─────┘
      │       │             │
      │       │             └── Foreground color (4 bits)
      │       │                 0000 = Black
      │       │                 0010 = Green
      │       │                 0100 = Red
      │       │                 1110 = Yellow
      │       │                 1111 = White
      │       │
      │       └──────────────── Background color (3 bits)
      │
      └──────────────────────── Blink/Intensity bit
```

#### Color Values Used

| Color  | Value | Binary   | Description |
|--------|-------|----------|-------------|
| BLACK  | 0     | 0000     | Background default |
| GREEN  | 2     | 0010     | Success messages |
| RED    | 4     | 0100     | Error messages |
| YELLOW | 14    | 1110     | Highlights |
| WHITE  | 15    | 1111     | Normal text |

---

## Mandatory Part Implementation

### Boot Assembly ([`boot.asm`](mandatory/asm/boot.asm))

The assembly boot file is the first code that runs after GRUB loads our kernel.

```asm
bits 32                          ; We're in 32-bit protected mode

section .multiboot               ; Multiboot header section
        dd 0x1BADB002            ; Magic number
        dd 0x0                   ; Flags (none)
        dd - (0x1BADB002 + 0x0)  ; Checksum

section .text
global start                     ; Export 'start' symbol
extern main                      ; Import 'main' from C

start:
        cli                      ; Disable interrupts
        mov esp, stack_space     ; Set up stack pointer
        call main                ; Call C main function
        hlt                      ; Halt CPU when main returns

section .bss
resb 8192                        ; Reserve 8KB for stack
stack_space:                     ; Stack grows downward from here
```

#### Key Instructions Explained

| Instruction | Purpose |
|-------------|---------|
| `bits 32` | Tells NASM to generate 32-bit code |
| `dd` | Define Double-word (4 bytes) |
| `cli` | Clear Interrupt Flag - disables hardware interrupts |
| `mov esp, stack_space` | Initialize stack pointer to top of reserved space |
| `call main` | Transfer control to C kernel |
| `hlt` | Halt CPU (stops execution) |
| `resb 8192` | Reserve 8192 bytes of uninitialized space |

#### Why Disable Interrupts?

At this early stage, we haven't set up an **Interrupt Descriptor Table (IDT)**. If an interrupt occurs without proper handling, the CPU will triple-fault and reset. We disable interrupts with `cli` until we're ready to handle them (in later KFS projects).

### Linker Script ([`linker.ld`](mandatory/linker.ld))

The linker script controls how the final kernel binary is organized in memory.

```ld
OUTPUT_FORMAT(elf32-i386)        /* Output as 32-bit ELF */
ENTRY(start)                     /* Entry point is 'start' from boot.asm */

SECTIONS
{
    . = 1M;                      /* Load kernel at 1MB physical address */
    
    .text BLOCK(4K) : ALIGN(4K) /* Code section, 4KB aligned */
    {
        *(.multiboot)            /* Multiboot header FIRST */
        *(.text)                 /* Then all code */
    }
    
    .data : { *(.data) }         /* Initialized data */
    .bss  : { *(.bss)  }         /* Uninitialized data */
}
```

#### Why Load at 1MB?

```
Physical Memory Map (x86):
┌──────────────────────┐ 0x00000000
│    Real Mode IVT     │ Interrupt Vector Table
├──────────────────────┤ 0x00000400
│    BIOS Data Area    │
├──────────────────────┤ 0x00000500
│   Conventional RAM   │ Available for use
├──────────────────────┤ 0x0007FFFF
│    Video Memory      │ VGA, VRAM
├──────────────────────┤ 0x000A0000
│    ROM/Reserved      │ BIOS ROM
├──────────────────────┤ 0x000FFFFF
│ ═══════════════════  │ ← 1MB boundary
│                      │
│   KERNEL LOADED      │ ← We load here (safe!)
│      HERE            │
│                      │
└──────────────────────┘
```

The first 1MB of memory is reserved for legacy hardware and BIOS. By loading at 1MB, we avoid conflicts with this reserved area.

#### Section Alignment

The `.text` section is aligned to **4KB** (4096 bytes) boundaries. This is important for:
- CPU page table efficiency (pages are typically 4KB)
- Memory protection (can mark pages as executable/read-only)
- Performance (cache line alignment)

### Screen Driver

#### Header ([`screen.h`](mandatory/includes/screen.h))

```c
#ifndef SCREEN_H
# define SCREEN_H

# define ROWS_COUNT (25)         // VGA text mode rows
# define COLUMNS_COUNT (80)      // VGA text mode columns

// Global variables
extern unsigned short* screen_buffer;  // Pointer to VGA memory
extern unsigned int cursor_index;      // Current cursor position

# define VGA_ADDRESS 0xB8000     // VGA text mode memory address

// Color definitions
# define BLACK  0
# define GREEN  2
# define RED    4
# define YELLOW 14
# define WHITE  15

// Function prototypes
int print_char(char c, unsigned char color);
int print_str(const char *s, unsigned char color);
int print_new_line(void);
int clear_screen(void);
void set_cursor_position(int x, int y);
void print_str_at(const char *s, unsigned char color, int x, int y);
void print_centered(const char *s, unsigned char color, int row);

#endif
```

#### Implementation ([`screen.c`](mandatory/kernel/utils/screen.c))

##### Global Variables

```c
unsigned short* screen_buffer;   // Points to 0xB8000
unsigned int cursor_index = 0;   // Linear position in buffer
```

##### Character Output

```c
int print_char(char c, unsigned char color)
{
    // Combine character and color into 16-bit value
    // Low byte: ASCII character
    // High byte: color attribute
    screen_buffer[cursor_index] = c | (unsigned short)color << 8;
    cursor_index++;
    return 0;
}
```

**Memory Layout of One Character:**
```
       High Byte          Low Byte
    ┌─────────────┐   ┌─────────────┐
    │  Attribute  │   │  Character  │
    │   (color)   │   │   (ASCII)   │
    └─────────────┘   └─────────────┘
         ▲                  ▲
         │                  │
    color << 8              c
         │                  │
         └────────┬─────────┘
                  │
              c | (color << 8)
```

##### String Output

```c
int print_str(const char *s, unsigned char color)
{
    int index = 0;
    while (s[index])          // Until null terminator
    {
        print_char(s[index], color);
        index++;
    }
    return index;             // Return characters printed
}
```

##### New Line

```c
int print_new_line(void)
{
    // Calculate remaining columns in current row
    // Add that to cursor to move to next row start
    cursor_index += COLUMNS_COUNT - ((cursor_index) % COLUMNS_COUNT);
    return 0;
}
```

**How it works:**
```
Before: cursor_index = 45 (row 0, column 45)
        45 % 80 = 45 (current column)
        80 - 45 = 35 (remaining in row)
        45 + 35 = 80 (start of row 1)

Row 0:  [...............................*...............]
        Position 45 ─────────────────────▲
        
After:  cursor_index = 80 (row 1, column 0)
Row 1:  [*......................................................]
        ▲── Now at start of new row
```

##### Clear Screen

```c
int clear_screen(void)
{
    // Fill entire screen with zeros (black spaces)
    for (unsigned int i = 0; i < (ROWS_COUNT * COLUMNS_COUNT); i++) {
        screen_buffer[i] = 0;
    }
    cursor_index = 0;
    return 0;
}
```

##### Centered Text

```c
void print_centered(const char *s, unsigned char color, int row)
{
    int len = str_len(s);
    int x = (COLUMNS_COUNT - len) / 2;  // Calculate center offset
    if (x < 0) x = 0;
    
    set_cursor_position(x, row);
    print_str(s, color);
}
```

**Centering Calculation:**
```
Screen width: 80 columns
String "42": 2 characters
Center position: (80 - 2) / 2 = 39

Row 12: [                                       42                                       ]
        └───────────── 39 columns ─────────────┘└─┘└───────────── 39 columns ─────────────┘
```

### Kernel Entry Point ([`kernel.c`](mandatory/kernel/kernel.c))

```c
#include "screen.h"

int main(void)
{
    // Initialize screen buffer pointer
    screen_buffer = (unsigned short *)VGA_ADDRESS;
    
    // Clear any existing content
    clear_screen();
    
    // Display "42" centered on row 12 (middle of 25 rows)
    print_centered("42", WHITE, 12);
    
    return 0;
}
```

This minimal kernel:
1. Sets up access to VGA memory
2. Clears the screen
3. Displays "42" in the center

### Build System ([`Makefile`](mandatory/Makefile))

```makefile
ASM = nasm               # Assembler for boot.asm
CC = gcc                 # C compiler
LD = ld                  # Linker

INCLUDES_DIR = includes/

# Critical compilation flags for kernel development
CFLAGS = -m32 \                    # 32-bit code generation
         -fno-builtin \            # Don't use built-in functions
         -fno-stack-protector \    # No stack protection (no runtime)
         -nostdlib \               # No standard library
         -nodefaultlibs \          # No default libraries
         -I$(INCLUDES_DIR)

LDFLAGS = -m elf_i386 \            # 32-bit ELF output
          -T linker.ld             # Use our linker script
```

#### Compilation Flags Explained

| Flag | Purpose | Why Needed |
|------|---------|------------|
| `-m32` | Generate 32-bit code | i386 architecture requirement |
| `-fno-builtin` | Don't use GCC built-ins | No runtime library available |
| `-fno-stack-protector` | Disable stack canaries | Would call `__stack_chk_fail` |
| `-nostdlib` | Don't link libc | We have no standard library |
| `-nodefaultlibs` | Don't link default libs | Complete independence |

#### Build Process

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│   boot.asm   │────▶│   boot.o     │     │              │
└──────────────┘     └──────────────┘     │              │
     nasm -f elf32                        │              │
                                          │              │
┌──────────────┐     ┌──────────────┐     │   ld         │     ┌──────────────┐
│  kernel.c    │────▶│  kernel.o    │────▶│   linker.ld  │────▶│  kernel.bin  │
└──────────────┘     └──────────────┘     │              │     └──────────────┘
     gcc -c                               │              │
                                          │              │
┌──────────────┐     ┌──────────────┐     │              │
│  screen.c    │────▶│  screen.o    │     │              │
└──────────────┘     └──────────────┘     └──────────────┘
     gcc -c

                              │
                              ▼
                    ┌──────────────────┐
                    │ grub-mkrescue    │
                    └──────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │  my-kernel.iso   │
                    └──────────────────┘
```

---

## Bonus Part Implementation

### Multiple Virtual Screens

The bonus implementation adds support for 3 virtual screens that can be switched using keyboard shortcuts.

#### Data Structures

```c
#define SCREEN_COUNT 3
#define BUFFER_ROW_COUNT (ROWS_COUNT * 2)  // Double buffering for scroll

// Screen storage buffers (one per virtual screen)
unsigned short stock[SCREEN_COUNT][BUFFER_ROW_COUNT * COLUMNS_COUNT];

// Track which screen is active
int screen_index;

// Store cursor position for each screen
unsigned short stock_cursor_index[SCREEN_COUNT];

// Track total rows written (for scroll calculation)
unsigned int total_row[SCREEN_COUNT];

// Extra scroll offset (for scrolling back through history)
unsigned short extra_scroll[SCREEN_COUNT];
```

#### Screen Switching

```c
void switch_screen(int new_screen_index)
{
    if (new_screen_index < 0 || new_screen_index >= SCREEN_COUNT)
        return;

    // Save current cursor position
    stock_cursor_index[screen_index] = cursor_index;
    
    // Switch to new screen
    screen_index = new_screen_index;
    
    // Restore cursor for new screen
    cursor_index = stock_cursor_index[screen_index];
    
    // Display the new screen's content
    display_screen(screen_index);
}
```

**Memory Organization:**
```
stock[0][ ... ]  ← Screen 1 buffer (4000+ bytes)
stock[1][ ... ]  ← Screen 2 buffer (4000+ bytes)
stock[2][ ... ]  ← Screen 3 buffer (4000+ bytes)
        │
        ▼
   Copy to VGA memory (0xB8000) when switching
```

### Keyboard Input Handling

#### I/O Port Communication

The keyboard communicates through specific I/O ports:

```c
#define KEYBOARD_DATA_PORT   0x60   // Read key data
#define KEYBOARD_STATUS_PORT 0x64   // Check key availability
```

#### Reading from I/O Ports (Assembly)

```c
static inline unsigned char inb(unsigned short port) {
    unsigned char value;
    // Read byte from port into 'value'
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
```

**Assembly Breakdown:**
- `inb` - Input Byte instruction
- `%1` - Port number (input operand)
- `%0` - Destination (output operand)
- `"=a"(value)` - Output to 'value' via AL register
- `"Nd"(port)` - Port number in DX register or immediate

#### Scancode Tables

Keyboards send **scancodes**, not ASCII characters. We need translation tables:

```c
const char scancode_lowercase[] = {
    0,  27, '1', '2', '3', '4', '5', '6',  // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=', '\b',    // 0x08 - 0x0E (backspace)
    '\t', 'q', 'w', 'e', 'r', 't', 'y',    // 0x0F - 0x15 (tab)
    'u', 'i', 'o', 'p', '[', ']', '\n',    // 0x16 - 0x1C (enter)
    0,  'a', 's', 'd', 'f', 'g', 'h',      // 0x1D - 0x23 (ctrl)
    'j', 'k', 'l', ';', '\'', '`', 0,      // 0x24 - 0x2A (left shift)
    '\\','z', 'x', 'c', 'v', 'b', 'n',     // 0x2B - 0x31
    'm', ',', '.', '/', 0, '*', 0, ' ',    // 0x32 - 0x39 (space)
};

const char scancode_uppercase[] = {
    // Shifted versions: '1' becomes '!', 'a' becomes 'A', etc.
    0,  27, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b',
    // ... etc
};
```

#### Keyboard Handler Loop

```c
void handle_keyboard()
{
    int shift_pressed = 0;
    int ctrl_pressed = 0;

    while (1)  // Infinite loop - kernel never exits
    {
        if (new_key_event())  // Check if key available
        {
            unsigned char scancode = get_scancode();
            
            // Handle arrow keys (extended scancodes)
            if (scancode == 0xE0) {
                scancode = get_scancode();
                if (scancode == 0x48)       // Up arrow
                    scroll_up();
                else if (scancode == 0x50)  // Down arrow
                    scroll_down();
            }
            // Handle modifier keys
            else if (scancode == PRESS_LEFT_SHIFT || scancode == PRESS_RIGHT_SHIFT)
                shift_pressed = 1;
            else if (scancode == RELEASE_LEFT_SHIFT || scancode == RELEASE_RIGHT_SHIFT)
                shift_pressed = 0;
            else if (scancode == PRESS_CTRL)
                ctrl_pressed = 1;
            else if (scancode == RELEASE_CTRL)
                ctrl_pressed = 0;
            // Handle Ctrl+1/2/3 for screen switching
            else if (ctrl_pressed) {
                char ascii = scancode_to_ascii(scancode, shift_pressed);
                if (ascii >= '1' && ascii < ('1' + SCREEN_COUNT))
                    switch_screen(ascii - '1');
            }
            // Handle regular key press (ignore releases)
            else if (!(scancode & 0x80)) {
                char ascii = scancode_to_ascii(scancode, shift_pressed);
                print_char(ascii, WHITE);
            }
        }
    }
}
```

**Scancode Press/Release:**
```
Key Press:   0x1E (A key pressed)
Key Release: 0x9E (A key released) = 0x1E | 0x80

The release scancode has bit 7 set (0x80 = 10000000 binary)
```

### Scroll Support

The scroll system maintains a buffer larger than the visible screen, allowing users to scroll back through history.

```c
void scroll_screen()
{
    // Calculate how many rows have overflowed past the screen
    unsigned int overflow_rows = total_row[screen_index] - ROWS_COUNT + 1;

    // Adjust for manual scroll offset
    if (overflow_rows > 0) {
        overflow_rows = extra_scroll[screen_index] > overflow_rows 
                        ? 0 
                        : overflow_rows - extra_scroll[screen_index];
    }
    
    // Calculate starting position in buffer
    unsigned int start = overflow_rows * COLUMNS_COUNT;
    
    // Copy from buffer to visible VGA memory
    for (unsigned int i = 0; i < (ROWS_COUNT * COLUMNS_COUNT); i++)
    {
        screen_buffer[i] = stock[screen_index][start + i];
    }
    
    // Update cursor position
    set_cursor_offset(last_char_index + 1);
}
```

**Scroll Buffer Visualization:**
```
stock[screen_index] buffer (50 rows):
┌────────────────────────────────────┐ Row 0 (oldest)
│  Previous output...                │
├────────────────────────────────────┤ Row 1
│  More output...                    │
├────────────────────────────────────┤
│         ...                        │
├────────────────────────────────────┤ Row 24
│                                    │
│  ┌──────────────────────────────┐  │ ← Visible window (25 rows)
│  │  Current visible content     │  │
│  │                              │  │
│  │  ...                         │  │
│  │                              │  │
│  └──────────────────────────────┘  │
├────────────────────────────────────┤ Row 49 (newest)
│  Latest output...                  │
└────────────────────────────────────┘

Arrow Up: Move window up (see older content)
Arrow Down: Move window down (see newer content)
```

### Hardware Cursor Control

The VGA hardware cursor is controlled through I/O ports:

```c
static inline void outb(int port, int value) {
    asm volatile ("outb %%al, %%dx" : : "a"(value), "d"(port));
}

void set_cursor_offset(int offset)
{
    // VGA CRTC (CRT Controller) registers
    // Port 0x3D4: Index register (select which register to write)
    // Port 0x3D5: Data register (write the value)
    
    outb(0x3D4, 0x0F);                    // Select cursor low byte register
    outb(0x3D5, (uint8_t)(offset & 0xFF)); // Write low byte
    
    outb(0x3D4, 0x0E);                    // Select cursor high byte register
    outb(0x3D5, (uint8_t)(offset >> 8));  // Write high byte
}
```

**Cursor Position Calculation:**
```
Position = (row × COLUMNS_COUNT) + column
         = (row × 80) + column

Example: Row 5, Column 10
Position = (5 × 80) + 10 = 410
```

### Printf Implementation

The bonus part includes a custom `kprintf()` function for formatted output, similar to the standard C `printf()`.

#### Custom Variable Arguments (No stdarg.h)

Since we can't use standard library headers, we implement our own variadic argument handling:

```c
// Custom type definitions
typedef char** va_list;
typedef unsigned int size_t;
typedef unsigned char uint8_t;
// ... etc

// Macro to calculate aligned size
#define VA_SIZE(type) ((sizeof(type) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1))

// Start iterating through arguments
#define va_start(ap, last) (*ap = (char*)(&last + 1))

// Get next argument of specified type
#define va_arg(ap, type) (*(type *)((*ap += VA_SIZE(type)) - VA_SIZE(type)))

// End iteration
#define va_end(ap) (*ap = NULL)
```

**How va_arg Works (Stack Layout):**
```
Stack (growing downward):
┌────────────────────┐ High address
│    Argument N      │
├────────────────────┤
│    Argument 2      │
├────────────────────┤
│    Argument 1      │
├────────────────────┤
│  'last' parameter  │ ← va_start points here + 1
├────────────────────┤
│   Return address   │
├────────────────────┤
│   Previous EBP     │
└────────────────────┘ Low address

va_start: Points ap to first argument after 'last'
va_arg:   Returns current and advances ap by type size
```

#### Format Specifiers Supported

| Specifier | Type | Example |
|-----------|------|---------|
| `%s` | String | `kprintf("Hello %s", "World")` |
| `%c` | Character | `kprintf("Char: %c", 'A')` |
| `%d`, `%i` | Signed integer | `kprintf("Num: %d", 42)` |
| `%u` | Unsigned integer | `kprintf("Unsigned: %u", 42)` |
| `%x` | Hex (lowercase) | `kprintf("Hex: %x", 255)` → "ff" |
| `%X` | Hex (uppercase) | `kprintf("Hex: %X", 255)` → "FF" |
| `%p` | Pointer address | `kprintf("Ptr: %p", ptr)` → "0x..." |
| `%%` | Literal % | `kprintf("100%%")` → "100%" |

#### Main Printf Function

```c
int kprintf(const char *format, ...)
{
    t_sc sc;                    // Structure to track output
    va_list arg;
    
    initialized_structure(&sc);
    va_start(arg, format);
    
    while (*format)
    {
        if (*format == '%')
            format = ft_read_arg(format + 1, &sc, arg);  // Process specifier
        else
            format = ft_read_text(format, &sc);          // Print regular text
            
        if (!format) {
            va_end(arg);
            print_str("(null)", WHITE);
            return (-1);
        }
    }
    
    va_end(arg);
    return ((int)sc.len);       // Return total characters printed
}
```

---

## Technical Deep Dive

### Memory Map Summary

```
Physical Address Space (x86 32-bit):
┌─────────────────────────────────────┐ 0xFFFFFFFF (4GB)
│                                     │
│     Extended Memory                 │
│     (Available for kernel)          │
│                                     │
├─────────────────────────────────────┤ 0x00100000 (1MB)
│     Kernel loaded here              │ ← Our kernel
├─────────────────────────────────────┤ 0x000F0000
│     BIOS ROM                        │
├─────────────────────────────────────┤ 0x000C0000
│     Video ROM                       │
├─────────────────────────────────────┤ 0x000B8000
│     VGA Text Mode Memory            │ ← screen_buffer points here
├─────────────────────────────────────┤ 0x000A0000
│     Video Memory                    │
├─────────────────────────────────────┤ 0x0009FFFF
│                                     │
│     Conventional Memory             │
│     (640KB available)               │
│                                     │
├─────────────────────────────────────┤ 0x00000500
│     BIOS Data Area                  │
├─────────────────────────────────────┤ 0x00000400
│     Interrupt Vector Table          │
└─────────────────────────────────────┘ 0x00000000
```

### Why These Specific Compilation Flags?

#### `-fno-builtin`
GCC will sometimes replace calls like `strlen()` with optimized built-in versions. These built-ins might call other library functions we don't have.

```c
// Without -fno-builtin, GCC might optimize this:
for (int i = 0; str[i]; i++);

// Into a call to built-in strlen, which might need memset, etc.
```

#### `-fno-stack-protector`
Stack protection adds "canary" values to detect buffer overflows:

```c
// With stack protection:
void function() {
    __stack_chk_guard = random_value;  // Set canary
    // ... function code ...
    if (__stack_chk_guard != random_value)
        __stack_chk_fail();  // We don't have this function!
}
```

#### `-nostdlib` and `-nodefaultlibs`
These prevent linking with:
- libc (standard C library)
- libgcc (GCC runtime support)
- crt0.o (C runtime startup code)

We must provide everything ourselves.

### ELF Binary Format

Our kernel is compiled as an **ELF32** (Executable and Linkable Format) binary:

```
ELF32 Header Structure:
┌─────────────────────────────────────┐
│  e_ident[16]    Magic number etc    │
├─────────────────────────────────────┤
│  e_type         ET_EXEC (2)         │  Executable
│  e_machine      EM_386 (3)          │  i386
│  e_version      1                   │
│  e_entry        'start' address     │  Entry point
│  e_phoff        Program header off  │
│  e_shoff        Section header off  │
│  ...                                │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│         Program Headers             │
│   (describe loadable segments)      │
├─────────────────────────────────────┤
│         .text section               │
│   (multiboot header + code)         │
├─────────────────────────────────────┤
│         .data section               │
│   (initialized global data)         │
├─────────────────────────────────────┤
│         .bss section                │
│   (uninitialized data - stack)      │
└─────────────────────────────────────┘
```

---

## Building and Running

### Prerequisites

- **GCC** (with 32-bit support: `gcc-multilib`)
- **NASM** (Netwide Assembler)
- **GNU LD** (Linker)
- **GRUB** (grub-mkrescue, grub-pc-bin)
- **xorriso** (for ISO creation)
- **KVM/QEMU** (for testing)

### Build Commands

#### Mandatory Part
```bash
cd kfs1/mandatory

# Build everything
make all

# Run in KVM
make run

# Clean build files
make clean

# Rebuild from scratch
make re
```

#### Bonus Part
```bash
cd kfs1/bonus

# Build everything (including printf library)
make all

# Run in KVM
make run

# Clean all
make clean
```

### Testing with QEMU/KVM

```bash
# Basic run
kvm -boot d -cdrom ./iso/my-kernel.iso -m 512

# With debugging
qemu-system-i386 -cdrom ./iso/my-kernel.iso -m 512 -d int,cpu_reset

# With GDB debugging
qemu-system-i386 -cdrom ./iso/my-kernel.iso -m 512 -s -S
# Then in another terminal:
gdb -ex "target remote :1234" -ex "symbol-file iso/boot/kernel.bin"
```

### Expected Output

#### Mandatory Part
```
┌────────────────────────────────────────────────────────────────────────────────┐
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                       42                                       │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
│                                                                                │
└────────────────────────────────────────────────────────────────────────────────┘
```

#### Bonus Part
```
┌────────────────────────────────────────────────────────────────────────────────┐
│Welcome to screen 1 !                                                           │
│█                                                                               │
│                                                                                │
│                                   - Cursor blinks here                         │
│                                   - Type to see characters                     │
│                                   - Ctrl+1/2/3 to switch screens               │
│                                   - Arrow Up/Down to scroll                    │
│                                                                                │
└────────────────────────────────────────────────────────────────────────────────┘
```

---

## References

### Official Documentation
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [OSDev Wiki - Main Page](https://wiki.osdev.org/Main_Page)
- [OSDev Wiki - Bare Bones](https://wiki.osdev.org/Bare_Bones)
- [OSDev Wiki - Printing to Screen](https://wiki.osdev.org/Printing_To_Screen)

### VGA Programming
- [OSDev Wiki - VGA Hardware](https://wiki.osdev.org/VGA_Hardware)
- [OSDev Wiki - Text Mode Cursor](https://wiki.osdev.org/Text_Mode_Cursor)

### Keyboard Programming
- [OSDev Wiki - PS/2 Keyboard](https://wiki.osdev.org/PS/2_Keyboard)
- [Keyboard Scancodes](https://wiki.osdev.org/Keyboard#Scan_Code_Set_1)

### Linker Scripts
- [GNU LD Manual - Scripts](https://sourceware.org/binutils/docs/ld/Scripts.html)
- [OSDev Wiki - Linker Scripts](https://wiki.osdev.org/Linker_Scripts)

### x86 Assembly
- [Intel x86 Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [NASM Documentation](https://www.nasm.us/xdoc/2.16.01/html/nasmdoc0.html)

---

## Summary

KFS-1 establishes the foundation for operating system development:

| Component | Purpose | Key Files |
|-----------|---------|-----------|
| **Boot** | Initialize CPU state, set up stack, call kernel | `boot.asm` |
| **Linker** | Organize binary layout, place at 1MB | `linker.ld` |
| **Screen** | VGA text mode output | `screen.c`, `screen.h` |
| **Kernel** | Main program logic | `kernel.c` |
| **[Bonus] Keyboard** | Input handling | `keyboard.c` |
| **[Bonus] Scroll** | Buffer management | `scroll.c` |
| **[Bonus] Printf** | Formatted output | `printf/*.c` |

This project teaches:
1. How computers boot (BIOS → GRUB → Kernel)
2. Multiboot protocol for bootloader compatibility
3. Direct hardware access (VGA memory-mapped I/O)
4. Freestanding C programming (no standard library)
5. Custom build systems for kernel development
6. I/O port communication for keyboard input

**Next Step:** KFS-2 will introduce the **Global Descriptor Table (GDT)** for memory segmentation and protection.