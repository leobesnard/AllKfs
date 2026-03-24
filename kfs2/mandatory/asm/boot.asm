;; =============================================================================
;; boot.asm - Multiboot-compliant Kernel Bootloader
;; =============================================================================
;;
;; This assembly file provides the multiboot header and entry point required
;; for GRUB to load and execute our kernel. It sets up the initial execution
;; environment including the GDT and kernel stack.
;;
;; Boot Sequence:
;;   1. GRUB loads kernel image into memory
;;   2. _start is called with multiboot info in registers
;;   3. Interrupts are disabled for initialization
;;   4. GDT is initialized via C code
;;   5. GDTR is loaded with the descriptor table address
;;   6. Stack pointer is configured
;;   7. Control transfers to C main() function
;;
;; =============================================================================

bits 32

;; =============================================================================
;; MULTIBOOT HEADER SECTION
;; =============================================================================
;; The multiboot header must be located within the first 8KB of the kernel
;; image. GRUB searches for this magic signature to verify boot compatibility.

section .multiboot
        dd 0x1BADB002                   ; Multiboot magic number
        dd 0x0                          ; Flags (no special features requested)
        dd -(0x1BADB002 + 0x0)          ; Checksum (magic + flags + checksum = 0)

;; =============================================================================
;; BSS SECTION - Uninitialized Data
;; =============================================================================
;; Reserve space for the kernel stack. The stack grows downward, so
;; kernel_stack_top marks the initial stack pointer value (highest address).

section .bss
        resb 8192                       ; Reserve 8KB for kernel stack
        global kernel_stack_top         ; Export symbol for C code access
        kernel_stack_top:

;; =============================================================================
;; DATA SECTION - Initialized Data
;; =============================================================================
;; Contains the GDT descriptor structure loaded via LGDT instruction.

section .data
        gdtr_descriptor:                ; GDTR structure (6 bytes)
            dw (7 * 8) - 1              ; Limit: GDT size minus 1 (7 entries * 8 bytes)
            dd 0x00000800               ; Base: Linear address of GDT (0x800)

;; =============================================================================
;; TEXT SECTION - Executable Code
;; =============================================================================

section .text
        extern main, gdt_init, stack_display    ; C function references
        global start                            ; Export entry point symbol

;; -----------------------------------------------------------------------------
;; start - Kernel Entry Point
;; -----------------------------------------------------------------------------
;; Entry point called by GRUB bootloader. Initializes protected mode
;; environment and transfers control to C kernel code.

start:
        cli                             ; Disable interrupts during initialization
        call gdt_init                   ; Initialize GDT entries via C function
        lgdt [gdtr_descriptor]          ; Load GDT descriptor into GDTR register
        mov esp, kernel_stack_top       ; Initialize kernel stack pointer
        call main                       ; Transfer control to C main function
        hlt                             ; Halt CPU (should not return here)
