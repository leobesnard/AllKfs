# KFS-3 Makefile
# Memory Management Kernel

ASM = nasm
CC = gcc
LD = ld

# Directories
INCLUDES_DIR = includes/
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

# Output
KERNEL_BIN = $(BOOT_DIR)/kernel.bin
ISO_FILE = $(ISO_DIR)/kfs3.iso

# Compilation flags
# -m32: Generate 32-bit code
# -fno-builtin: Don't use built-in functions
# -fno-exceptions: Disable C++ exceptions (for C++ safety)
# -fno-stack-protector: Disable stack protector (no __stack_chk_fail)
# -nostdlib: Don't use standard library
# -nodefaultlibs: Don't use default libraries
# -Wall: Enable all warnings
# -Wextra: Enable extra warnings
# -ffreestanding: Freestanding environment
CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs \
         -Wall -Wextra -ffreestanding -I$(INCLUDES_DIR)

# Linker flags
LDFLAGS = -m elf_i386 -T linker.ld

# Source files
ASM_SRC = asm/boot.asm

KERNEL_SRC = kernel/kernel.c

UTILS_SRC = kernel/utils/screen.c \
            kernel/utils/str.c \
            kernel/utils/gdt.c

MEMORY_SRC = kernel/memory/pmm.c \
             kernel/memory/paging.c \
             kernel/memory/vmm.c \
             kernel/memory/kmalloc.c

PANIC_SRC = kernel/panic/panic.c

# Object files
ASM_OBJ = boot.o
KERNEL_OBJ = kernel.o
UTILS_OBJ = screen.o str.o gdt.o
MEMORY_OBJ = pmm.o paging.o vmm.o kmalloc.o
PANIC_OBJ = panic.o

ALL_OBJ = $(ASM_OBJ) $(KERNEL_OBJ) $(UTILS_OBJ) $(MEMORY_OBJ) $(PANIC_OBJ)

# Default target
all: $(ISO_FILE)

# Run in QEMU/KVM
run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 512

# Run with debug output
debug: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 512 -d int,cpu_reset -no-reboot

# Create ISO
$(ISO_FILE): $(KERNEL_BIN)
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)

# Link kernel
$(KERNEL_BIN): $(ALL_OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(ALL_OBJ)

# Compile assembly
$(ASM_OBJ): $(ASM_SRC)
	$(ASM) -f elf32 -o $(ASM_OBJ) $(ASM_SRC)

# Compile kernel main
$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c -o $(KERNEL_OBJ) $(KERNEL_SRC)

# Compile utilities
screen.o: kernel/utils/screen.c
	$(CC) $(CFLAGS) -c -o screen.o kernel/utils/screen.c

str.o: kernel/utils/str.c
	$(CC) $(CFLAGS) -c -o str.o kernel/utils/str.c

gdt.o: kernel/utils/gdt.c
	$(CC) $(CFLAGS) -c -o gdt.o kernel/utils/gdt.c

# Compile memory management
pmm.o: kernel/memory/pmm.c
	$(CC) $(CFLAGS) -c -o pmm.o kernel/memory/pmm.c

paging.o: kernel/memory/paging.c
	$(CC) $(CFLAGS) -c -o paging.o kernel/memory/paging.c

vmm.o: kernel/memory/vmm.c
	$(CC) $(CFLAGS) -c -o vmm.o kernel/memory/vmm.c

kmalloc.o: kernel/memory/kmalloc.c
	$(CC) $(CFLAGS) -c -o kmalloc.o kernel/memory/kmalloc.c

# Compile panic
$(PANIC_OBJ): $(PANIC_SRC)
	$(CC) $(CFLAGS) -c -o $(PANIC_OBJ) $(PANIC_SRC)

# Clean
clean:
	rm -f *.o $(KERNEL_BIN) $(ISO_FILE)

# Rebuild
re: clean all

# Check file size
size: $(KERNEL_BIN)
	@echo "Kernel size:"
	@ls -la $(KERNEL_BIN)
	@echo ""
	@echo "ISO size:"
	@ls -la $(ISO_FILE) 2>/dev/null || echo "ISO not built yet"

.PHONY: all run debug clean re size
