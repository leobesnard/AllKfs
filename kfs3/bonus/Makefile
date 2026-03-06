# KFS-3 Bonus Makefile
# Memory Management with Shell

ASM = nasm
CC = gcc
LD = ld

INCLUDES_DIR = includes/
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

KERNEL_BIN = $(BOOT_DIR)/kernel.bin
ISO_FILE = $(ISO_DIR)/kfs3-bonus.iso

CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs \
         -Wall -Wextra -ffreestanding -I$(INCLUDES_DIR)

LDFLAGS = -m elf_i386 -T linker.ld

ASM_SRC = asm/boot.asm

# Source files
KERNEL_SRC = kernel/kernel.c

UTILS_SRC = kernel/utils/screen.c \
            kernel/utils/str.c \
            kernel/utils/gdt.c \
            kernel/utils/keyboard.c

MEMORY_SRC = kernel/memory/pmm.c \
             kernel/memory/paging.c \
             kernel/memory/vmm.c \
             kernel/memory/kmalloc.c

PANIC_SRC = kernel/panic/panic.c

SHELL_SRC = kernel/shell/shell.c

# Object files
ASM_OBJ = boot.o
KERNEL_OBJ = kernel.o
UTILS_OBJ = screen.o str.o gdt.o keyboard.o
MEMORY_OBJ = pmm.o paging.o vmm.o kmalloc.o
PANIC_OBJ = panic.o
SHELL_OBJ = shell.o

ALL_OBJ = $(ASM_OBJ) $(KERNEL_OBJ) $(UTILS_OBJ) $(MEMORY_OBJ) $(PANIC_OBJ) $(SHELL_OBJ)

all: $(ISO_FILE)

run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 512

debug: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 512 -d int,cpu_reset -no-reboot

$(ISO_FILE): $(KERNEL_BIN)
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)

$(KERNEL_BIN): $(ALL_OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(ALL_OBJ)

$(ASM_OBJ): $(ASM_SRC)
	$(ASM) -f elf32 -o $(ASM_OBJ) $(ASM_SRC)

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c -o $(KERNEL_OBJ) $(KERNEL_SRC)

screen.o: kernel/utils/screen.c
	$(CC) $(CFLAGS) -c -o screen.o kernel/utils/screen.c

str.o: kernel/utils/str.c
	$(CC) $(CFLAGS) -c -o str.o kernel/utils/str.c

gdt.o: kernel/utils/gdt.c
	$(CC) $(CFLAGS) -c -o gdt.o kernel/utils/gdt.c

keyboard.o: kernel/utils/keyboard.c
	$(CC) $(CFLAGS) -c -o keyboard.o kernel/utils/keyboard.c

pmm.o: kernel/memory/pmm.c
	$(CC) $(CFLAGS) -c -o pmm.o kernel/memory/pmm.c

paging.o: kernel/memory/paging.c
	$(CC) $(CFLAGS) -c -o paging.o kernel/memory/paging.c

vmm.o: kernel/memory/vmm.c
	$(CC) $(CFLAGS) -c -o vmm.o kernel/memory/vmm.c

kmalloc.o: kernel/memory/kmalloc.c
	$(CC) $(CFLAGS) -c -o kmalloc.o kernel/memory/kmalloc.c

$(PANIC_OBJ): $(PANIC_SRC)
	$(CC) $(CFLAGS) -c -o $(PANIC_OBJ) $(PANIC_SRC)

$(SHELL_OBJ): $(SHELL_SRC)
	$(CC) $(CFLAGS) -c -o $(SHELL_OBJ) $(SHELL_SRC)

clean:
	rm -f *.o $(KERNEL_BIN) $(ISO_FILE)

re: clean all

size: $(KERNEL_BIN)
	@echo "Kernel size:"
	@ls -la $(KERNEL_BIN)
	@echo ""
	@echo "ISO size:"
	@ls -la $(ISO_FILE) 2>/dev/null || echo "ISO not built yet"

.PHONY: all run debug clean re size
