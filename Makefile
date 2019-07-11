CC = gcc
CFLAGS = -m32 -nostdlib -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf -I bootsect

all: bbnOS

include bootsect/Makefile

bbnOS: ${BOOT_BIN}
	cp ${BOOT_BIN} output/os.bin

clean:
	rm -rf output/bootsect/*.*