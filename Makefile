OBJECTS: boot.bin
CC = gcc
CFLAGS = -m32 -nostdlib -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: boot.bin

boot.o: boot.asm
	$(AS) $(ASFLAGS) boot.asm -o boot.o

boot.bin: boot.o
	ld -T link16.ld boot.o -o boot.elf
	objcopy -O binary boot.elf boot.bin

clean:
	rm -rf *.o boot.elf boot.bin