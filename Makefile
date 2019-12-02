BUILD_DIR = build
LOADER_DIR = bootsect

LOADER_ASM_SOURCES = ${wildcard ${LOADER_DIR}/*.asm}
LOADER_ASM_OBJS = ${patsubst ${LOADER_DIR}/%.asm, ${BUILD_DIR}/${LOADER_DIR}/%.obj, ${LOADER_ASM_SOURCES}}	# change .asm to .o
KERN_OBJS = ${BUILD_DIR}/kernel.elf

ISO_DIR = ${BUILD_DIR}/iso
ISO_OUT = ${BUILD_DIR}/os.iso
BOOT_BIN = ${BUILD_DIR}/os.bin
BOOT_COM = ${BUILD_DIR}/os.com

ASM = nasm
ASM_FLAGS = -f elf32
LOADER_ASM_FLAGS = -f elf32 -Ibootsect
CC = gcc
CC_FLAGS = -c -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
-Wall -Wextra
LD = ld
LOADER_LD_FLAGS = -T bootsect/link16.ld -melf_i386

FREEDOS_ISO = deps/FLOPPY.img
BOOT_FOR_DOS = deps/floppy_boot.img
MOUNT_DIR = deps/mntfloppy

all: bin

# Optional: QEMU
qemu: iso
	qemu-system-i386 -cdrom ${ISO_OUT}

qemu_gdb: iso
	qemu-system-i386 -cdrom ${ISO_OUT} -s -S

qemu_nogra: iso
	qemu-system-i386 -cdrom ${ISO_OUT} -nographic

qemu_nogra_gdb: iso
	qemu-system-i386 -cdrom ${ISO_OUT} -nographic -s -S

# freebsd
freedos: com ${FREEDOS_ISO} ${BOOT_FOR_DOS}
	mkdir -p ${MOUNT_DIR}
	sudo mount -o loop ${BOOT_FOR_DOS} ${MOUNT_DIR}
	sudo cp ${BOOT_COM} ${MOUNT_DIR}
	sudo umount ${MOUNT_DIR}
	qemu-system-i386 -fda ${FREEDOS_ISO} -fdb ${BOOT_FOR_DOS}

# make COM file
com: bin
	cp ${BOOT_BIN} ${BOOT_COM}

# make kernel binary
bin: prepare ${KERN_OBJS} 
	objcopy -O binary ${KERN_OBJS} ${BOOT_BIN}

# linking here
${KERN_OBJS}: ${LOADER_ASM_OBJS}
	${LD} ${LOADER_LD_FLAGS} $^ -o $@ 

# compile .asm files to .o files
${LOADER_ASM_OBJS}: ${BUILD_DIR}/${LOADER_DIR}/%.obj: ${LOADER_DIR}/%.asm
	${ASM} ${LOADER_ASM_FLAGS} -o $@ $<

# make build directory
prepare:
	mkdir -p ${BUILD_DIR} ${BUILD_DIR}/${LOADER_DIR}

clean:
	rm -rf build