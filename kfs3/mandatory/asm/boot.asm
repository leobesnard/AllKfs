bits 32

;;;;;;;;;;; SECTION MULTIBOOT ;;;;;;;;;;;;  
section .multiboot
    MULTIBOOT_MAGIC     equ 0x1BADB002
    MULTIBOOT_ALIGN     equ 1 << 0          ; Align loaded modules on page boundaries
    MULTIBOOT_MEMINFO   equ 1 << 1          ; Provide memory map
    MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO
    MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

;;;;;;;;;;; SECTION BSS ;;;;;;;;;;;;  
section .bss
    align 16
    stack_bottom:
        resb 16384              ; 16KB for kernel stack
    global stack_top
    stack_top:

;;;;;;;;;;; SECTION DATA ;;;;;;;;;;;;  
section .data
    global gdt_descriptor
    gdt_descriptor:
        dw (7 * 8) - 1          ; GDT size - 1 (7 entries * 8 bytes each)
        dd 0x00000800           ; GDT address (0x800)

;;;;;;;;;;; SECTION TEXT ;;;;;;;;;;;;  
section .text
    extern main, setup_gdt
    global start
    global stack_top

start:
    cli                         ; Disable interrupts

    ; Save multiboot information
    mov [multiboot_magic], eax
    mov [multiboot_info], ebx

    ; Setup GDT first
    call setup_gdt
    lgdt [gdt_descriptor]

    ; Reload segment registers with new GDT selectors
    jmp 0x08:.reload_cs         ; Far jump to reload CS (kernel code segment)

.reload_cs:
    mov ax, 0x10                ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ax, 0x18                ; Kernel stack segment selector
    mov ss, ax

    ; Setup stack
    mov esp, stack_top

    ; Push multiboot info for kernel main
    push dword [multiboot_info]
    push dword [multiboot_magic]

    ; Call kernel main
    call main

    ; If main returns, halt
    cli
.hang:
    hlt
    jmp .hang

section .data
    global multiboot_magic
    global multiboot_info
    multiboot_magic: dd 0
    multiboot_info:  dd 0
