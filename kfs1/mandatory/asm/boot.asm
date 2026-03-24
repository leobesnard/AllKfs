; **************************************************************************** ;
;                                                                              ;
;   boot.asm - Multiboot-compliant kernel bootstrap code                       ;
;                                                                              ;
; **************************************************************************** ;

bits 32

; ----------------------------------------------------------------------------
; Multiboot Header Section
; The bootloader (GRUB) searches for this magic signature to identify
; the kernel as multiboot-compliant
; ----------------------------------------------------------------------------
section .multiboot
    dd 0x1BADB002                ; Multiboot magic number
    dd 0x0                       ; Flags (no special requirements)
    dd - (0x1BADB002 + 0x0)      ; Checksum (magic + flags + checksum = 0)

; ----------------------------------------------------------------------------
; Text Section - Executable code
; ----------------------------------------------------------------------------
section .text

global start
extern main                      ; C kernel entry point

; Kernel entry point - called by bootloader
start:
    cli                          ; Disable interrupts during initialization
    mov esp, kernel_stack_top    ; Initialize stack pointer
    call main                    ; Transfer control to C kernel
    hlt                          ; Halt processor on return

; ----------------------------------------------------------------------------
; BSS Section - Uninitialized data (kernel stack)
; ----------------------------------------------------------------------------
section .bss
    resb 8192                    ; Reserve 8KB for kernel stack
kernel_stack_top:
