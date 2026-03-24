;==============================================================================
;                           KFS2 BONUS - BOOT LOADER
;==============================================================================
; Multiboot-compliant bootloader entry point
; Sets up GDT, stack, and transfers control to the C kernel
;==============================================================================

bits 32

;==============================================================================
;                           MULTIBOOT HEADER SECTION
;==============================================================================
; The multiboot header must be in the first 8KB of the kernel image
; GRUB looks for this magic sequence to identify the kernel

section .multiboot
        dd 0x1BADB002               ; Multiboot magic number
        dd 0x0                      ; Flags (none set)
        dd - (0x1BADB002 + 0x0)     ; Checksum (magic + flags + checksum = 0)

;==============================================================================
;                           BSS SECTION - KERNEL STACK
;==============================================================================
; Reserve 8KB for the kernel stack
; Stack grows downward, so kernel_stack_top marks the initial ESP

section .bss
        resb 8192                   ; Reserve 8KB for kernel stack
        global kernel_stack_top     ; Export symbol for C code access
        kernel_stack_top:

;==============================================================================
;                           DATA SECTION - GDT DESCRIPTOR
;==============================================================================
; GDT register descriptor structure for LGDT instruction
; Contains the size and base address of the GDT

section .data
        gdtr_descriptor:            ; GDT register descriptor
            dw (7 * 8) - 1          ; GDT limit: 7 entries * 8 bytes - 1
            dd 0x00000800           ; GDT base address (0x800)

;==============================================================================
;                           TEXT SECTION - ENTRY POINT
;==============================================================================

section .text
        extern main, gdt_init, stack_display   ; C functions
        global start                            ; Entry point symbol

;------------------------------------------------------------------------------
; start - Kernel entry point
;------------------------------------------------------------------------------
; This is where GRUB transfers control after loading the kernel.
; We set up the GDT, initialize the stack, and call the C main function.
;------------------------------------------------------------------------------

start:
        cli                         ; Disable interrupts during setup

        ; Load the Global Descriptor Table
        lgdt [gdtr_descriptor]

        ; Set up the kernel stack pointer
        mov esp, kernel_stack_top

        ; Call the C kernel main function
        call main

        ; If main returns, halt the CPU
        hlt
