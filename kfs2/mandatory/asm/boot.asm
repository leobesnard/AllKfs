bits 32

;;;;;;;;;;; SECTION MULTIBOOT ;;;;;;;;;;;;  
section .multiboot              
        dd 0x1BADB002            ;set magic number for bootloader
        dd 0x0                   ;set flags
        dd - (0x1BADB002 + 0x0)  ; Checksum

;;;;;;;;;;; SECTION BSS ;;;;;;;;;;;;  
section .bss
        resb 8192                ; 8KB pour la pile
        global stack_space       ; Rendre stack_space accessible depuis C
        stack_space:

;;;;;;;;;;; SECTION DATA ;;;;;;;;;;;;
section .data
        gdt_descriptor:          ; Descripteur de la GDT (taille et adresse)
            dw (7 * 8) - 1       ; Taille de la GDT - 1 (7 entrées de 8 octets)
            dd 0x00000800        ; Adresse de la GDT (0x800)

;;;;;;;;;;; SECTION TEXT ;;;;;;;;;;;;  
section .text
        extern main,  setup_gdt, print_kernel_stack       ;defined in the C file               
        global start                                      ;point d'entrée

start:
        cli                          ; Block interrupts
        ;call main
        ;call print_kernel_stack
        call setup_gdt               ; Initialiser la GDT
        lgdt [gdt_descriptor]        ; Charger la GDT
        mov esp, stack_space         ; Configurer le pointeur de pile (kernel stack)
        call main                    ; Appeler la fonction principale
        hlt                          ; Arrêter le CPU