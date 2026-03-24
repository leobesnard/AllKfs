; ============================================================================
;
;   boot.asm - Multiboot-compliant kernel entry point
;
;   This is the initial entry point for the kernel. It sets up the stack
;   and transfers control to the C kernel main function.
;
; ============================================================================

bits 32

; ============================================================================
;   Multiboot Header Section
;
;   The multiboot header must be present for GRUB to recognize this as
;   a valid kernel. It contains the magic number, flags, and checksum.
; ============================================================================

section .multiboot
    dd 0x1BADB002                       ; Multiboot magic number
    dd 0x0                              ; Flags (none set)
    dd -(0x1BADB002 + 0x0)              ; Checksum (must sum to zero)

; ============================================================================
;   Text Section - Code Entry Point
; ============================================================================

section .text
global start
extern main                             ; External C function

start:
    cli                                 ; Disable interrupts
    mov esp, kernel_stack_top           ; Initialize stack pointer
    call main                           ; Call C kernel entry point
    hlt                                 ; Halt CPU (should never reach here)

; ============================================================================
;   BSS Section - Uninitialized Data (Kernel Stack)
; ============================================================================

section .bss
resb 32768                              ; Reserve 32KB for kernel stack
kernel_stack_top:                       ; Stack grows downward, so this is the top
