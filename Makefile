CC=gcc
LD=gcc

# -fno-stack-protector убирает ошибку __stack_chk_fail_local
# -fno-builtin запрещает GCC подменять циклы встроенными функциями деления
# -fno-pic отключает генерацию позиционно-независимого кода, ломающего адреса ядра
CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-builtin -fno-pic -fno-pie -mno-sse -mno-mmx

# -Wl,--no-warn-rwx-segments отключает предупреждение линкера о правах сегментов
# -Wl,-z,max-page-size=0x1000 жестко выравнивает секции по стандарту Multiboot (4КБ)
LDFLAGS=-m32 -T linker.ld -ffreestanding -nostdlib -no-pie -Wl,--no-warn-rwx-segments -Wl,-z,max-page-size=0x1000

all: os.iso

boot.o: boot.asm
	nasm -f elf32 boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

shell.o: shell/shell.c
	$(CC) $(CFLAGS) -c shell/shell.c -o shell.o

commands.o: shell/commands.c
	$(CC) $(CFLAGS) -c shell/commands.c -o commands.o

keyboard.o: drivers/keyboard.c
	$(CC) $(CFLAGS) -c drivers/keyboard.c -o keyboard.o

kernel.bin: boot.o kernel.o shell.o commands.o keyboard.o
	$(LD) $(LDFLAGS) \
	boot.o \
	kernel.o \
	shell.o \
	commands.o \
	keyboard.o \
	-o kernel.bin

os.iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso

run: os.iso
	qemu-system-i386 -m 64M -cdrom os.iso -d int,guest_errors,cpu_reset -no-reboot -no-shutdown

clean:
	rm -rf *.o *.bin *.iso iso
	rm -f shell/*.o
	rm -f drivers/*.o

