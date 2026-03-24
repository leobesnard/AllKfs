# KFS-2: GDT (Global Descriptor Table) & Stack

## 📚 Table of Contents

1. [Project Overview](#project-overview)
2. [Prerequisites from KFS-1](#prerequisites-from-kfs-1)
3. [Core Concepts In Depth](#core-concepts-in-depth)
   - [Understanding Memory Segmentation](#understanding-memory-segmentation)
   - [What is the GDT?](#what-is-the-gdt)
   - [Segment Descriptors Explained](#segment-descriptors-explained)
   - [The Stack: A Deep Dive](#the-stack-a-deep-dive)
   - [Privilege Levels (Rings)](#privilege-levels-rings)
4. [Mandatory Part Implementation](#mandatory-part-implementation)
   - [Boot Assembly Changes](#boot-assembly-changes)
   - [GDT Entry Structure](#gdt-entry-structure)
   - [Creating GDT Descriptors](#creating-gdt-descriptors)
   - [Loading the GDT](#loading-the-gdt)
   - [Stack Printing Tool](#stack-printing-tool)
5. [Bonus Part Implementation](#bonus-part-implementation)
   - [Shell Implementation](#shell-implementation)
   - [Command Execution](#command-execution)
   - [System Commands](#system-commands)
6. [Technical Deep Dive](#technical-deep-dive)
   - [Access Byte Breakdown](#access-byte-breakdown)
   - [Granularity Byte Breakdown](#granularity-byte-breakdown)
   - [Flat Memory Model](#flat-memory-model)
7. [Building and Running](#building-and-running)
8. [Debug and Testing](#debug-and-testing)
9. [Common Issues and Solutions](#common-issues-and-solutions)
10. [References](#references)

---

## Project Overview

KFS-2 builds upon KFS-1 by introducing the **Global Descriptor Table (GDT)**, a critical data structure for x86 processors that defines memory segments and their access rights. This project also implements tools to inspect the kernel stack.

### What You'll Learn

1. **Memory Segmentation**: How x86 CPUs organize and protect memory regions
2. **Privilege Levels**: The ring system (Ring 0-3) for kernel/user separation
3. **GDT Structure**: How to define, fill, and load a GDT
4. **Stack Management**: Understanding how the call stack works at a low level
5. **LGDT Instruction**: Loading the GDT into the CPU's GDTR register

### Objectives Achieved

- ✅ Created a Global Descriptor Table with 7 entries
- ✅ Implemented all required segments (Kernel Code/Data/Stack, User Code/Data/Stack)
- ✅ GDT placed at required address 0x00000800
- ✅ GDT properly loaded using LGDT instruction
- ✅ Kernel stack printing tool (human-readable format)
- ✅ [Bonus] Interactive shell with commands
- ✅ [Bonus] System commands: halt, reboot, shutdown

---

## Prerequisites from KFS-1

This project builds on top of KFS-1. You should have:

```
From KFS-1:
├── Bootable kernel via GRUB (Multiboot compliant)
├── VGA text mode driver (screen output)
├── Basic kernel library (strlen, strcmp, etc.)
└── Build system (Makefile, linker script)
```

The GDT extends this foundation by adding memory protection and segment management.

---

## Core Concepts In Depth

### Understanding Memory Segmentation

#### Historical Context: Why Segmentation Exists

In the early days of x86 (8086 processor, 1978), CPUs could only address 1MB of memory using 20-bit addresses, but registers were only 16 bits wide. **Segmentation** was invented to solve this:

```
Physical Address = (Segment Register × 16) + Offset

Example:
Segment = 0x1000
Offset  = 0x0500
Physical = 0x1000 × 16 + 0x0500 = 0x10000 + 0x0500 = 0x10500
```

#### Real Mode vs Protected Mode

| Mode | Bits | Address Space | Segmentation |
|------|------|---------------|--------------|
| **Real Mode** | 16-bit | 1 MB | Simple (segment × 16 + offset) |
| **Protected Mode** | 32-bit | 4 GB | Complex (GDT defines segments) |

When GRUB loads our kernel, it switches the CPU to **Protected Mode**. In this mode, segment registers no longer contain physical addresses - they contain **selectors** that point to entries in the GDT.

```
Real Mode:                          Protected Mode:
                                    
┌──────────────┐                    ┌──────────────┐
│ Segment Reg  │ = Physical Base    │ Segment Reg  │ = Selector
└──────────────┘                    └──────┬───────┘
       │                                   │
       ▼                                   ▼
┌──────────────┐                    ┌──────────────┐
│  + Offset    │                    │     GDT      │
└──────────────┘                    │  ┌────────┐  │
       │                            │  │Entry 0 │  │
       ▼                            │  ├────────┤  │
┌──────────────┐                    │  │Entry 1 │◄─┼── Selector points here
│Physical Addr │                    │  ├────────┤  │
└──────────────┘                    │  │Entry 2 │  │
                                    │  └────────┘  │
                                    └──────────────┘
                                           │
                                           ▼
                                    ┌──────────────┐
                                    │ Base + Limit │
                                    │  + Access    │
                                    └──────────────┘
```

### What is the GDT?

The **Global Descriptor Table (GDT)** is a data structure that tells the CPU about memory segments:

- **Where** each segment starts (Base Address)
- **How big** each segment is (Limit)
- **What access rights** apply (Read/Write/Execute, Privilege Level)
- **What type** of segment it is (Code, Data, System)

#### Why Do We Need a GDT?

1. **Memory Protection**: Prevent user programs from accessing kernel memory
2. **Privilege Separation**: Different code runs at different privilege levels
3. **Code/Data Separation**: Mark memory as executable or data-only
4. **Required by CPU**: The x86 CPU in Protected Mode requires a valid GDT

#### GDT Location Requirement

The subject requires the GDT to be at address **0x00000800**:

```
Physical Memory:
┌────────────────────┐ 0x00000000
│  Interrupt Vector  │
│      Table         │
├────────────────────┤ 0x00000400
│    BIOS Data       │
├────────────────────┤ 0x00000500
│     Free Area      │
├────────────────────┤ 0x00000800 ← GDT placed here!
│        GDT         │   (7 entries × 8 bytes = 56 bytes)
│   (56 bytes)       │
├────────────────────┤ 0x00000838
│     Free Area      │
├────────────────────┤
│        ...         │
```

### Segment Descriptors Explained

Each GDT entry is exactly **8 bytes (64 bits)** and has a complex structure:

```
Bit Layout of a GDT Entry (8 bytes):
┌─────────────────────────────────────────────────────────────────────────────┐
│        Byte 7       │        Byte 6       │    Byte 5    │    Byte 4       │
├─────────────────────┼─────────────────────┼──────────────┼─────────────────┤
│   Base (24-31)      │ Flags │Limit(16-19) │    Access    │  Base (16-23)   │
│     8 bits          │ 4 bits│   4 bits    │    8 bits    │    8 bits       │
├─────────────────────┴───────┴─────────────┴──────────────┴─────────────────┤
│        Byte 3       │        Byte 2       │    Byte 1    │    Byte 0       │
├─────────────────────┼─────────────────────┼──────────────┴─────────────────┤
│   Base (8-15)       │   Base (0-7)        │         Limit (0-15)           │
│     8 bits          │     8 bits          │            16 bits             │
└─────────────────────┴─────────────────────┴────────────────────────────────┘
```

#### Why is the Structure So Fragmented?

The weird layout (base and limit split across non-contiguous bits) is due to **backward compatibility** with the 80286 processor. Intel needed 32-bit protected mode to be compatible with the 16-bit protected mode of the 286.

#### Structure in C Code

```c
// From gdt.c - Packed structure to match exact memory layout
struct GDTEntry {
    unsigned short limit_low;     // Limit bits 0-15  (16 bits)
    unsigned short base_low;      // Base bits 0-15   (16 bits)
    unsigned char  base_middle;   // Base bits 16-23  (8 bits)
    unsigned char  access;        // Access byte      (8 bits)
    unsigned char  granularity;   // Flags + Limit 16-19 (8 bits)
    unsigned char  base_high;     // Base bits 24-31  (8 bits)
} __attribute__((packed));        // CRITICAL: No padding!
```

**Why `__attribute__((packed))`?**

Without this attribute, the compiler might add padding bytes for alignment:

```
Without packed (compiler may add padding):
struct {
    short a;      // 2 bytes
    // 2 bytes padding added here!
    int b;        // 4 bytes
}  // Total: 8 bytes instead of expected 6

With packed (exact layout):
struct {
    short a;      // 2 bytes
    int b;        // 4 bytes
} __attribute__((packed));  // Total: 6 bytes, exactly as specified
```

### The Stack: A Deep Dive

#### What is a Stack?

The stack is a region of memory used for:

1. **Function Call Management**: Storing return addresses
2. **Local Variables**: Temporary storage for function data
3. **Function Parameters**: Passing arguments between functions
4. **Saved Registers**: Preserving CPU state during calls

#### Stack Behavior: LIFO (Last In, First Out)

```
Stack Operations:

PUSH (add to stack):                 POP (remove from stack):
                                     
    Before    After                      Before    After
    ┌─────┐   ┌─────┐                    ┌─────┐   ┌─────┐
    │     │   │     │                    │     │   │     │
    │     │   │     │                    │     │   │     │
ESP─▶─────│   │     │                    │     │   │     │
    │ old │   │ old │                ESP─▶ new │   │ new │
    │     │   │     │                    │─────│   │─────│◀─ESP
    └─────┘ ESP────│◀─ESP                │ old │   │ old │
              │new │                     └─────┘   └─────┘
              └─────┘                    
    ESP decreases!                   ESP increases!
```

**Important**: On x86, the stack grows **downward** (toward lower addresses). When you PUSH, ESP decreases; when you POP, ESP increases.

#### Stack Frame Structure

When a function is called, a **stack frame** is created:

```
Function call: caller() calls callee(arg1, arg2)

High Address
┌─────────────────────────┐
│        arg2             │  ← Arguments pushed right-to-left
├─────────────────────────┤
│        arg1             │
├─────────────────────────┤
│    Return Address       │  ← Where to go after callee returns
├─────────────────────────┤
│    Saved EBP            │  ← Previous frame pointer (caller's EBP)
├────────────────────────┤◀─ EBP (frame pointer) points here
│    Local Variable 1     │
├─────────────────────────┤
│    Local Variable 2     │
├─────────────────────────┤◀─ ESP (stack pointer) points here
│         ...             │
Low Address

// Assembly for function prologue:
push ebp          ; Save caller's frame pointer
mov ebp, esp      ; Set new frame pointer
sub esp, N        ; Allocate space for local variables
```

#### Stack in Our Kernel

In [`boot.asm`](mandatory/asm/boot.asm:10), we reserve 8KB for the kernel stack:

```asm
section .bss
        resb 8192                ; Reserve 8KB (8192 bytes) for stack
        global stack_space       ; Export symbol for C code
        stack_space:             ; Label at TOP of reserved space
```

**Why does `stack_space` come AFTER `resb`?**

Because the stack grows downward! `stack_space` is at the highest address of the reserved region:

```
Reserved Stack Space:
                                    
Address:  0x????0000               
          ┌─────────────────┐      
          │                 │      
          │  8KB Reserved   │  ← Stack grows INTO this space
          │     Space       │      
          │                 │      
          └─────────────────┘      
Address:  0x????2000 ◀─ stack_space (ESP starts here)
                                    
When we do: mov esp, stack_space
ESP points to the highest address, and grows DOWN into the reserved space.
```

### Privilege Levels (Rings)

The x86 architecture has 4 privilege levels, called **rings**:

```
                    ┌───────────────────────┐
                    │       Ring 3          │  User Applications
                    │    (Least Trusted)    │  (Browsers, Games, etc.)
                    └───────────────────────┘
                              │
               ┌──────────────┴──────────────┐
               ▼                             ▼
        ┌─────────────┐               ┌─────────────┐
        │   Ring 2    │               │   Ring 1    │  Device Drivers
        │  (Unused)   │               │  (Unused)   │  (Usually merged with Ring 0)
        └─────────────┘               └─────────────┘
                    │                       │
                    └───────────┬───────────┘
                                ▼
                    ┌───────────────────────┐
                    │       Ring 0          │  Kernel
                    │    (Most Trusted)     │  (Full hardware access)
                    └───────────────────────┘

Modern OS typically only use Ring 0 (kernel) and Ring 3 (user).
```

#### Why Privilege Levels Matter

| Ring | Name | Can Do | Cannot Do |
|------|------|--------|-----------|
| **0** | Kernel | Everything | N/A |
| **3** | User | Run code, access own memory | Access hardware directly, access kernel memory |

The GDT defines which ring each segment belongs to, enforcing these restrictions.

---

## Mandatory Part Implementation

### Boot Assembly Changes

#### Original KFS-1 boot.asm:
```asm
start:
    cli
    mov esp, stack_space
    call main
    hlt
```

#### KFS-2 boot.asm with GDT:

```asm
;;;;;;;;;;; SECTION DATA ;;;;;;;;;;;;
section .data
        gdt_descriptor:          ; GDT Descriptor (6 bytes)
            dw (7 * 8) - 1       ; Size: 7 entries × 8 bytes - 1 = 55
            dd 0x00000800        ; Base: Address where GDT is stored

;;;;;;;;;;; SECTION TEXT ;;;;;;;;;;;;  
section .text
        extern main, setup_gdt, print_kernel_stack
        global start

start:
        cli                      ; Disable interrupts
        call setup_gdt           ; Fill GDT entries in memory
        lgdt [gdt_descriptor]    ; Load GDT into GDTR register
        mov esp, stack_space     ; Setup kernel stack
        call main                ; Call kernel main
        hlt                      ; Halt CPU
```

#### The GDT Descriptor Structure

The `lgdt` instruction expects a specific 6-byte structure:

```
GDT Descriptor (GDTR format):
┌─────────────────────────────────────────┐
│  Offset 0-1: Size (16 bits)             │  Size of GDT - 1
├─────────────────────────────────────────┤
│  Offset 2-5: Base Address (32 bits)     │  Physical address of GDT
└─────────────────────────────────────────┘

In our code:
    dw (7 * 8) - 1       ; 7 entries × 8 bytes = 56 - 1 = 55 (0x37)
    dd 0x00000800        ; GDT at address 0x800 (as required)
```

**Why Size - 1?**

The size field stores the **last valid byte offset**, not the total size.
- 7 entries × 8 bytes = 56 bytes total
- Last valid byte is at offset 55 (0 through 55)
- So we store 55

### GDT Entry Structure

From [`gdt.c`](mandatory/kernel/utils/gdt.c:6):

```c
struct GDTEntry {
    unsigned short limit_low;     // Limit (bits 0-15)
    unsigned short base_low;      // Base (bits 0-15)
    unsigned char  base_middle;   // Base (bits 16-23)
    unsigned char  access;        // Access byte
    unsigned char  granularity;   // Granularity + Limit (bits 16-19)
    unsigned char  base_high;     // Base (bits 24-31)
} __attribute__((packed));
```

#### Visual Byte Mapping:

```
Memory Layout of One GDT Entry (8 bytes):

Byte:    0      1      2      3      4      5      6      7
      ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
      │limit │limit │base  │base  │base  │access│gran+ │base  │
      │ low  │ low  │ low  │ low  │middle│      │limit │ high │
      │(0-7) │(8-15)│(0-7) │(8-15)│(16-23)│      │(16-19)│(24-31)│
      └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
         └─────────┘   └─────────┘
          limit_low      base_low
```

### Creating GDT Descriptors

From [`gdt.c`](mandatory/kernel/utils/gdt.c:107):

```c
void create_descriptor(int index, unsigned int base, unsigned int limit, 
                       unsigned char access, unsigned char gran) {
    // Split the 32-bit base address across 3 fields
    gdt[index].base_low = (base & 0xFFFF);           // Bits 0-15
    gdt[index].base_middle = (base >> 16) & 0xFF;   // Bits 16-23
    gdt[index].base_high = (base >> 24) & 0xFF;     // Bits 24-31

    // Split the 20-bit limit across 2 fields
    gdt[index].limit_low = (limit & 0xFFFF);         // Bits 0-15
    gdt[index].granularity = ((limit >> 16) & 0x0F); // Bits 16-19 (low nibble)

    // Combine granularity flags with limit high bits
    gdt[index].granularity |= gran & 0xF0;           // Flags in high nibble
    gdt[index].access = access;
}
```

#### Bit Manipulation Explained:

```
Example: base = 0x12345678

base & 0xFFFF:
    0x12345678
  & 0x0000FFFF
  = 0x00005678  → base_low = 0x5678

(base >> 16) & 0xFF:
    0x12345678 >> 16 = 0x00001234
  & 0x000000FF
  = 0x00000034  → base_middle = 0x34

(base >> 24) & 0xFF:
    0x12345678 >> 24 = 0x00000012
  & 0x000000FF  
  = 0x00000012  → base_high = 0x12
```

### GDT Setup - All 7 Entries

From [`gdt.c`](mandatory/kernel/utils/gdt.c:123):

```c
void setup_gdt() {
    // Entry 0: NULL Descriptor (required by CPU)
    create_descriptor(0, 0, 0, 0, 0);
    
    // Entry 1: Kernel Code Segment
    create_descriptor(1, 0, 0xFFFFF, 0x9A, 0xC0);
    
    // Entry 2: Kernel Data Segment
    create_descriptor(2, 0, 0xFFFFF, 0x92, 0xC0);
    
    // Entry 3: Kernel Stack Segment
    create_descriptor(3, 0, 0xFFFFF, 0x92, 0xC0);
    
    // Entry 4: User Code Segment
    create_descriptor(4, 0, 0xFFFFF, 0xFA, 0xC0);
    
    // Entry 5: User Data Segment
    create_descriptor(5, 0, 0xFFFFF, 0xF2, 0xC0);
    
    // Entry 6: User Stack Segment
    create_descriptor(6, 0, 0xFFFFF, 0xF2, 0xC0);
}
```

#### Entry-by-Entry Breakdown:

| Index | Offset | Name | Base | Limit | Access | Granularity | Purpose |
|-------|--------|------|------|-------|--------|-------------|---------|
| 0 | 0x00 | NULL | 0 | 0 | 0x00 | 0x00 | Required null entry |
| 1 | 0x08 | Kernel Code | 0 | 0xFFFFF | 0x9A | 0xC0 | Execute kernel code |
| 2 | 0x10 | Kernel Data | 0 | 0xFFFFF | 0x92 | 0xC0 | Read/write kernel data |
| 3 | 0x18 | Kernel Stack | 0 | 0xFFFFF | 0x92 | 0xC0 | Kernel call stack |
| 4 | 0x20 | User Code | 0 | 0xFFFFF | 0xFA | 0xC0 | Execute user programs |
| 5 | 0x28 | User Data | 0 | 0xFFFFF | 0xF2 | 0xC0 | User program data |
| 6 | 0x30 | User Stack | 0 | 0xFFFFF | 0xF2 | 0xC0 | User program stack |

**Offset Calculation**: Each entry is 8 bytes, so:
- Entry 1 offset = 1 × 8 = 0x08
- Entry 2 offset = 2 × 8 = 0x10
- Entry 3 offset = 3 × 8 = 0x18
- etc.

### Loading the GDT

After filling the GDT entries in memory, we load it into the CPU:

```asm
lgdt [gdt_descriptor]
```

This instruction loads the GDT descriptor into the **GDTR** (GDT Register):

```
GDTR Register (48 bits):
┌────────────────────────────────────────────────────────┐
│  Limit (16 bits)  │       Base Address (32 bits)       │
├───────────────────┼────────────────────────────────────┤
│       55          │           0x00000800               │
└───────────────────┴────────────────────────────────────┘

After lgdt:
- CPU knows GDT is at address 0x00000800
- CPU knows GDT contains 56 bytes (55 + 1)
- All segment lookups will use this GDT
```

#### Reading Back with SGDT

We can verify the GDT was loaded correctly:

```c
struct {
    unsigned short limit;
    unsigned int base;
} gdt_ptr;

// Store current GDTR value into gdt_ptr
asm volatile("sgdt %0" : "=m" (gdt_ptr));

// Now gdt_ptr.base should be 0x00000800
// And gdt_ptr.limit should be 55 (0x37)
```

### Stack Printing Tool

From [`gdt.c`](mandatory/kernel/utils/gdt.c:67):

```c
void print_kernel_stack() {
    int *stack_ptr;
    int stack_base = (int)&stack_space;  // Top of stack (from boot.asm)
    int value;
 
    // Get current ESP value using inline assembly
    asm volatile("movl %%esp, %0" : "=r"(stack_ptr));

    print_str("---- KERNEL STACK ---- ", RED);
    print_new_line();
    
    // Print stack metadata
    print_str("Stack Address: ", WHITE);
    print_hex((unsigned int)stack_ptr, LIGHT_CYAN);  // Current ESP
    print_new_line();
    
    print_str("Stack Base: ", WHITE);
    print_hex(stack_base, LIGHT_CYAN);               // Top of stack
    print_new_line();
    
    print_str("Stack Size: ", WHITE);
    print_hex(stack_base - (unsigned int)stack_ptr, LIGHT_CYAN);  // Used bytes
    print_str(" bytes", LIGHT_CYAN);
    print_new_line();

    // Print stack contents (top 4-10 values)
    print_str("---- STACK CONTENT ---- ", RED);
    print_new_line();

    for (int i = 0; i < 4 && stack_ptr < (int*)stack_base; i++) {
        print_str("Stack[", LIGHT_GREEN);
        print_hex((unsigned int)stack_ptr, LIGHT_GREEN);
        print_str("]: ", LIGHT_GREEN);
        
        value = *stack_ptr;  // Read value at current stack position
        print_hex(value, WHITE);
        
        stack_ptr++;         // Move to next stack slot
        print_new_line();
    }
}
```

#### Understanding the Stack Walk:

```
When print_kernel_stack() is called:

         Stack Memory
         ┌─────────────────┐ High Address (stack_base)
         │                 │
         │   Empty Space   │ 
         │   (unused)      │
         │                 │
         ├─────────────────┤
         │ Return Address  │ ← stack_ptr[3] (where to return after main)
         ├─────────────────┤
         │ Saved Registers │ ← stack_ptr[2]
         ├─────────────────┤
         │ Local Variables │ ← stack_ptr[1]
         ├─────────────────┤
ESP ───▶ │ Current Frame   │ ← stack_ptr[0] (current ESP position)
         └─────────────────┘ Low Address

The loop walks UP the stack (increasing addresses) to show
what's been pushed onto the stack.
```

---

## Bonus Part Implementation

### Shell Implementation

The bonus adds an interactive shell with debugging commands.

#### Shell Prompt

From [`screen.c`](bonus/kernel/utils/screen.c:215):

```c
void new_prompt() {
    print_new_line();
    print_str("kfs2", WHITE);     // Kernel name
    print_str("@", GREEN);         // Separator
    print_str("screen", WHITE);    // Terminal identifier
    kprintf("%d", screen_index + 1);  // Screen number (1, 2, or 3)
    print_str("> ", GREEN);        // Command prompt
}

// Output: "kfs2@screen1> "
```

#### Keyboard Handler with Command Processing

From [`keyboard.c`](bonus/kernel/utils/keyboard.c:60):

```c
void handle_keyboard() {
    int shift_pressed = 0;
    int ctrl_pressed = 0;

    while (1) {
        if (new_key_event()) {
            unsigned char scancode = get_scancode();
            
            // Handle special keys...
            
            else if (!(scancode & RELEASE_MASK)) {
                char ascii = scancode_to_ascii(scancode, shift_pressed);
                
                if (ascii == '\n') {
                    // ENTER pressed - execute command!
                    const int result = exec_cmd();
                    new_prompt();  // Show new prompt
                } else {
                    print_char(ascii, WHITE);  // Echo typed character
                }
            }
        }
    }
}
```

### Command Execution

From [`screen.c`](bonus/kernel/utils/screen.c:251):

```c
int exec_cmd() {
    char cmdcpy[CMD_BUFFER_SIZE + 1];  // Buffer for command (60 chars max)
    int cmd_start_index = 1;

    // Walk backward from cursor to find command start
    // (Stop at the green '>' character from prompt)
    while (cmd_start_index <= CMD_BUFFER_SIZE && 
           stock[screen_index][cursor_index - cmd_start_index] != 
           (' ' | (unsigned short)GREEN<<8)) {
        // Copy character (extract low byte from screen buffer)
        cmdcpy[CMD_BUFFER_SIZE - cmd_start_index] = 
            *((char *)(&(stock[screen_index][cursor_index - cmd_start_index])));
        cmd_start_index++;
    }
    cmdcpy[CMD_BUFFER_SIZE] = 0;  // Null terminate
    
    // Compare extracted command against known commands
    char *cmd = cmdcpy + CMD_BUFFER_SIZE - cmd_start_index + 1;
    
    if (!kstrcmp(cmd, "print-stack")) {
        print_kernel_stack();
        return 1;
    }
    if (!kstrcmp(cmd, "gdt")) {
        print_gdt();
        return 2;
    }
    if (!kstrcmp(cmd, "halt")) {
        halt();
        return 3;
    }
    if (!kstrcmp(cmd, "reboot")) {
        reboot();
        return 4;
    }
    if (!kstrcmp(cmd, "shutdown")) {
        shutdown();
        return 5;
    }
    return 0;  // Unknown command
}
```

#### Command Extraction Process:

```
Screen Buffer when user types "gdt":

           Cursor position (after typing)
                    │
                    ▼
... │k│f│s│2│@│s│c│r│e│e│n│1│>│ │g│d│t│
                          ▲ GREEN     ▲
                          │           │
                     Stop here    Start walking back
                     (prompt)    from cursor

The algorithm:
1. Start at cursor position
2. Walk backward byte by byte
3. Stop when hitting the green '>' character
4. Reverse to get actual command string
```

### System Commands

#### halt() - Stop the CPU

```c
void halt(void) {
    __asm__ __volatile__("cli; hlt");
}
```

**Explanation:**
- `cli` - Clear Interrupt Flag (disable interrupts)
- `hlt` - Halt instruction (CPU enters low-power state)
- CPU will not wake up since interrupts are disabled

#### reboot() - Restart the System

```c
void reboot(void) {
    uint8_t good = 0x02;
    
    // Wait for keyboard controller to be ready
    while (good & 0x02)
        good = inb(0x64);  // Read keyboard status port
    
    // Send reset command to keyboard controller
    outb(0x64, 0xFE);
    
    // If reboot fails, halt
    halt();
}
```

**Explanation:**
The 8042 keyboard controller has a pin connected to the CPU reset line. Sending command 0xFE tells it to pulse that line, causing a system reset.

```
Port 0x64: Keyboard Controller Command Port
Command 0xFE: Pulse reset line

       ┌─────────────────┐
       │  8042 Keyboard  │
       │   Controller    │
       │                 │
0x64 ──▶│  Command Port  │───┐
       │                 │   │
       └─────────────────┘   │
                             │ 0xFE = Pulse Reset
                             ▼
       ┌─────────────────┐
       │    CPU Reset    │
       │      Line       │
       └─────────────────┘
                │
                ▼
            REBOOT!
```

#### shutdown() - Power Off (QEMU/Bochs)

```c
void shutdown(void) {
    outw(0x604, 0x2000);
}
```

**Explanation:**
This uses QEMU/Bochs-specific ACPI shutdown. Port 0x604 is an emulator-specific power management port. Value 0x2000 triggers a power-off.

> **Note**: This won't work on real hardware - proper ACPI shutdown requires parsing ACPI tables and using the proper shutdown port for the specific system.

---

## Technical Deep Dive

### Access Byte Breakdown

The access byte (8 bits) controls segment properties:

```
Access Byte Structure:
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ P │ DPL   │ S │    Type       │
│   │       │   │               │
│ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
└───┴───┴───┴───┴───┴───┴───┴───┘
  │   └──┬──┘  │   └─────┬─────┘
  │      │     │         │
  │      │     │         └── Type (4 bits): Segment type
  │      │     │             Code: Execute, Read, Conforming, Accessed
  │      │     │             Data: Expand-down, Write, Accessed
  │      │     │
  │      │     └── S (1 bit): Descriptor type
  │      │         1 = Code/Data segment
  │      │         0 = System segment (TSS, LDT, etc.)
  │      │
  │      └── DPL (2 bits): Descriptor Privilege Level
  │          00 = Ring 0 (Kernel)
  │          11 = Ring 3 (User)
  │
  └── P (1 bit): Present
      1 = Segment is in memory
      0 = Segment not present (will cause fault if accessed)
```

#### Access Byte Values Used:

| Value | Binary | Meaning |
|-------|--------|---------|
| **0x9A** | `10011010` | P=1, DPL=00, S=1, Type=1010 (Code, Execute/Read) |
| **0x92** | `10010010` | P=1, DPL=00, S=1, Type=0010 (Data, Read/Write) |
| **0xFA** | `11111010` | P=1, DPL=11, S=1, Type=1010 (Code, Execute/Read, Ring 3) |
| **0xF2** | `11110010` | P=1, DPL=11, S=1, Type=0010 (Data, Read/Write, Ring 3) |

**Detailed Breakdown of 0x9A (Kernel Code):**
```
0x9A = 1001 1010

  1   0   0   1     1   0   1   0
  │   └─┬─┘   │     │   │   │   │
  │     │     │     │   │   │   └── A (Accessed): 0 (CPU sets when accessed)
  │     │     │     │   │   └── R (Readable): 1 (Code can be read)
  │     │     │     │   └── C (Conforming): 0 (Non-conforming)
  │     │     │     └── E (Executable): 1 (This is code)
  │     │     └── S: 1 (Code/Data segment)
  │     └── DPL: 00 (Ring 0 - Kernel)
  └── P: 1 (Present in memory)
```

### Granularity Byte Breakdown

The granularity byte contains flags and the high bits of limit:

```
Granularity Byte:
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ G │D/B│ L │AVL│ Limit(16-19)  │
│   │   │   │   │               │
│ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
└───┴───┴───┴───┴───┴───┴───┴───┘
  │   │   │   │   └─────┬─────┘
  │   │   │   │         │
  │   │   │   │         └── Limit bits 16-19 (4 bits)
  │   │   │   │
  │   │   │   └── AVL (1 bit): Available for OS use
  │   │   │
  │   │   └── L (1 bit): Long mode (64-bit)
  │   │       0 = 32-bit segment
  │   │       1 = 64-bit segment (only in long mode)
  │   │
  │   └── D/B (1 bit): Default operation size
  │       Code: D=1 means 32-bit addresses/operands
  │       Data: B=1 means 32-bit stack pointer (ESP vs SP)
  │
  └── G (1 bit): Granularity
      0 = Limit is in bytes (max 1 MB)
      1 = Limit is in 4 KB pages (max 4 GB)
```

#### Granularity Value 0xC0:

```
0xC0 = 1100 0000

  1   1   0   0   0   0   0   0
  │   │   │   │   └─────┬─────┘
  │   │   │   │         │
  │   │   │   │         └── Limit high bits: 0000
  │   │   │   │             (Combined with limit_low = 0xFFFFF)
  │   │   │   │
  │   │   │   └── AVL: 0
  │   │   │
  │   │   └── L: 0 (Not 64-bit, we're in 32-bit protected mode)
  │   │
  │   └── D/B: 1 (32-bit default operation size)
  │
  └── G: 1 (4 KB granularity)

With G=1 and Limit=0xFFFFF:
Actual Limit = (0xFFFFF + 1) × 4KB - 1
             = 0x100000 × 0x1000 - 1
             = 0xFFFFFFFF
             = 4 GB (entire 32-bit address space)
```

### Flat Memory Model

Our GDT implements a **flat memory model**:

```
Flat Memory Model:
┌────────────────────────────────────────┐
│                                        │
│   All segments:                        │
│   • Base = 0x00000000                  │
│   • Limit = 0xFFFFF (with G=1 = 4GB)   │
│                                        │
│   Result: All segments cover           │
│   the entire 4GB address space!        │
│                                        │
└────────────────────────────────────────┘

Address Space:
0x00000000 ────────────────────────────────┐
           │                               │
           │   ┌───────────────────────┐   │
           │   │ Kernel Code (0x08)    │   │
           │   │ Kernel Data (0x10)    │   │
           │   │ Kernel Stack (0x18)   │───┼── All overlap
           │   │ User Code (0x20)      │   │   the same
           │   │ User Data (0x28)      │   │   4GB space!
           │   │ User Stack (0x30)     │   │
           │   └───────────────────────┘   │
           │                               │
0xFFFFFFFF ────────────────────────────────┘
```

**Why Flat Model?**

Modern operating systems use **paging** (covered in KFS-3) for memory protection instead of segmentation. The GDT is still required by the CPU, but we make all segments overlap so that segmentation is effectively disabled.

The actual memory protection is done by:
- **DPL (Descriptor Privilege Level)**: Kernel vs User separation
- **Paging**: Fine-grained memory protection (4KB pages)

---

## Building and Running

### Prerequisites

Same as KFS-1, plus:
- Understanding of hexadecimal and binary
- Knowledge of bit manipulation

### Build Commands

#### Mandatory Part
```bash
cd kfs2/mandatory

# Build everything
make all

# Run in KVM
make run

# Clean and rebuild
make re
```

#### Bonus Part
```bash
cd kfs2/bonus

# Build (includes printf library)
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

# With CPU state debugging
qemu-system-i386 -cdrom ./iso/my-kernel.iso -m 512 -d cpu

# With interrupt debugging
qemu-system-i386 -cdrom ./iso/my-kernel.iso -m 512 -d int

# Monitor mode (inspect GDT)
qemu-system-i386 -cdrom ./iso/my-kernel.iso -m 512 -monitor stdio
# Then type: info registers
# Or: x/56xb 0x800  (dump GDT bytes)
```

---

## Debug and Testing

### Verifying GDT Contents

In QEMU monitor (`-monitor stdio`):

```
(qemu) info registers
...
GDT=     00000800 00000037
...

(qemu) x/56xb 0x800
0x00000800: 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00  # NULL
0x00000808: 0xff 0xff 0x00 0x00 0x00 0x9a 0xcf 0x00  # Kernel Code
0x00000810: 0xff 0xff 0x00 0x00 0x00 0x92 0xcf 0x00  # Kernel Data
...
```

### Expected Output

#### Mandatory Part:
```
---- GDT Descriptors----
GDT Base Address: 0x00000800
GDT Size: 0x00000037

---- GDT Registres: ----
Null adress 0x00000800 | Access: 0x00000000
Kernel Code adress 0x00000808 | Access: 0x0000009A
Kernel Data adress 0x00000810 | Access: 0x00000092
Kernel Stack adress 0x00000818 | Access: 0x00000092
User Code adress 0x00000820 | Access: 0x000000FA
User Data adress 0x00000828 | Access: 0x000000F2
User Stack adress 0x00000830 | Access: 0x000000F2

---- KERNEL STACK ----
Stack Address: 0x00103xxx
Stack Base: 0x00104000
Stack Size: 0x00000xxx bytes

---- STACK CONTENT ----
Stack[0x00103xxx]: 0x001xxxxx
Stack[0x00103xxx]: 0x001xxxxx
...
```

#### Bonus Part Shell:
```
Welcome to screen 1 !
---- GDT Descriptors----
GDT Base Address: 0x00000800
...

kfs2@screen1> print-stack
---- KERNEL STACK ----
...

kfs2@screen1> gdt
---- GDT Registres: ----
...

kfs2@screen1> halt
(system halts)
```

---

## Common Issues and Solutions

### Issue 1: Triple Fault on Boot

**Symptom**: System continuously reboots

**Causes**:
1. GDT not loaded before switching to protected mode
2. Invalid GDT entry (wrong access byte)
3. Wrong GDT address

**Solution**: Ensure GDT is set up before `lgdt` and verify all entries.

### Issue 2: Page Fault When Accessing GDT

**Symptom**: Exception when reading GDT entries

**Cause**: GDT pointer not pointing to valid memory

**Solution**: Verify `GDT_ADRESS` is accessible and within first 1MB (where identity mapping exists).

### Issue 3: Stack Corruption

**Symptom**: Random values in print_kernel_stack

**Cause**: Stack not properly initialized

**Solution**: Make sure `mov esp, stack_space` happens after GDT setup and before calling any C functions.

### Issue 4: GRUB Error

**Symptom**: "Error loading kernel"

**Cause**: Kernel binary too large or invalid ELF

**Solution**: Verify `make all` completed successfully and check binary with `file iso/boot/kernel.bin`.

---

## References

### GDT Resources
- [OSDev Wiki - GDT](https://wiki.osdev.org/GDT)
- [OSDev Wiki - GDT Tutorial](https://wiki.osdev.org/GDT_Tutorial)
- [Intel SDM Vol. 3A, Chapter 3: Protected-Mode Memory Management](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

### Stack Resources
- [OSDev Wiki - Stack](https://wiki.osdev.org/Stack)
- [Call Stack](https://en.wikipedia.org/wiki/Call_stack)

### x86 Assembly
- [x86 Instruction Reference](https://www.felixcloutier.com/x86/)
- [LGDT Instruction](https://www.felixcloutier.com/x86/lgdt:lidt)
- [SGDT Instruction](https://www.felixcloutier.com/x86/sgdt)

### I/O Ports
- [OSDev Wiki - I/O Ports](https://wiki.osdev.org/I/O_Ports)
- [8042 Keyboard Controller](https://wiki.osdev.org/%228042%22_PS/2_Controller)

---

## Summary

KFS-2 introduces critical x86 protected mode concepts:

| Component | Purpose | Key Points |
|-----------|---------|------------|
| **GDT** | Memory segmentation | 7 entries, 8 bytes each, at 0x800 |
| **Segments** | Memory regions | Kernel (Ring 0) and User (Ring 3) segments |
| **Access Byte** | Permissions | Read/Write/Execute, Privilege Level |
| **Granularity** | Size control | 4KB pages for 4GB addressing |
| **LGDT/SGDT** | CPU register | Load/Store GDT descriptor |
| **Stack Tool** | Debugging | Inspect kernel stack contents |
| **[Bonus] Shell** | Interaction | Commands: gdt, print-stack, halt, reboot |

### Key Learnings:

1. **Flat Memory Model**: All segments cover 0-4GB, actual protection via paging (KFS-3)
2. **Privilege Levels**: Ring 0 (Kernel) vs Ring 3 (User) enforced by DPL
3. **Backward Compatibility**: GDT's weird structure due to 80286 compatibility
4. **Stack Growth**: x86 stack grows downward (PUSH decreases ESP)
5. **Assembly Integration**: `lgdt`, `sgdt`, `cli`, `hlt` instructions

**Next Step**: KFS-3 will introduce **Memory Management** (Paging and Virtual Memory).