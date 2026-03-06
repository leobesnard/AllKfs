ASM = nasm
CC = gcc
LD = ld

INCLUDES_DIR = ./includes/ -I./printf/include/

# Compilation en 32 bits (m32 pour l'architecture 32 bits)
CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs -I$(INCLUDES_DIR)
LDFLAGS = -m elf_i386 -T linker.ld -L./  --whole-archive libftprintf.a --no-whole-archive

BOOT_SRC = asm/boot.asm
KERNEL_SRC = kernel/kernel.c

UTILS_SRC = kernel/utils/screen.c

SCROLL_SRC = kernel/utils/scroll.c

KEYBOARD_SRC = kernel/utils/keyboard.c

# Répertoires de destination
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

# Noms de fichiers
KERNEL_BIN = $(BOOT_DIR)/kernel.bin
GRUB_CFG = $(GRUB_DIR)/grub.cfg

all: $(ISO_DIR)/my-kernel.iso

run: $(KERNEL_BIN)
	kvm -boot d -cdrom ./iso/my-kernel.iso -m 512

# Création de l'image ISO
$(ISO_DIR)/my-kernel.iso: $(KERNEL_BIN)
	grub-mkrescue -o $(ISO_DIR)/my-kernel.iso $(ISO_DIR)

# Compilation du noyau
$(KERNEL_BIN): libftprintf.a boot.o kernel.o screen.o scroll.o keyboard.o
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) boot.o kernel.o screen.o scroll.o keyboard.o

# Compilation de boot.asm en boot.o
# Compilation du bootloader :
boot.o: $(BOOT_SRC)
	$(ASM) -f elf32 -o boot.o $(BOOT_SRC)

# Compilation du noyau en kernel.o
kernel.o: $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c -o kernel.o $(KERNEL_SRC)

screen.o: $(UTILS_SRC)
	$(CC) $(CFLAGS) -c -o screen.o  $(UTILS_SRC)

scroll.o: $(SCROLL_SRC)
	$(CC) $(CFLAGS) -c -o scroll.o  $(SCROLL_SRC)

keyboard.o: $(KEYBOARD_SRC)
	$(CC) $(CFLAGS) -c -o keyboard.o  $(KEYBOARD_SRC)

libftprintf.a:
	make re -C printf
	mv printf/libftprintf.a ./libftprintf.a
# Nettoyage
clean:
	rm -f kernel/*.o kernel/utils/*.o $(KERNEL_BIN) $(ISO_DIR)/my-kernel.iso libftprintf.a
	make fclean -C printf

re: clean all