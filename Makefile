BUILD_DIR = build
BOOTSECT_DIR = bootsect

BOOT_ASM_SOURCES = ${BOOTSECT_DIR}/boot.asm
BOOT_ASM_OBJS = ${BUILD_DIR}/${BOOTSECT_DIR}/boot.obj	# change .asm to .o

LOADER_ASM_SOURCES = ${BOOTSECT_DIR}/loader.asm
LOADER_ASM_OBJS = ${BUILD_DIR}/${BOOTSECT_DIR}/loader.obj

BOOT_OBJS = ${BUILD_DIR}/boot.elf
LOADER_OBJS = ${BUILD_DIR}/loader.elf

ISO_DIR = ${BUILD_DIR}/iso
ISO_OUT = ${BUILD_DIR}/os.iso
BOOT_BIN = ${BUILD_DIR}/boot.bin
LOADER_BIN = ${BUILD_DIR}/loader.bin

ASM = nasm
ASM_FLAGS = -f elf32
BOOTSECT_ASM_FLAGS = -Ibootsect
CC = gcc
CC_FLAGS = -c -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
-Wall -Wextra
LD = ld
LD_FLAGS = -melf_i386

MOUNT_DIR = ${BUILD_DIR}/mnt
OUT_IMG = ${BUILD_DIR}/os.img

all: bin

# Optional: QEMU
qemu: iso
	qemu-system-i386 -fda ${OUT_IMG}

qemu_gdb: iso
	qemu-system-i386 -fda ${OUT_IMG} -s -S

qemu_nogra: iso
	qemu-system-i386 -fda ${OUT_IMG} -nographic

qemu_nogra_gdb: iso
	qemu-system-i386 -fda ${OUT_IMG} -nographic -s -S

# iso
iso: bin ${MOUNT_DIR}
	dd if=/dev/zero of=${OUT_IMG} bs=512 count=2880
	dd if=${BOOT_BIN} of=${OUT_IMG} bs=512 count=1 conv=notrunc
	sudo mount -o loop ${OUT_IMG} ${MOUNT_DIR}
	sudo cp ${LOADER_BIN} ${MOUNT_DIR} -v
	sudo umount ${MOUNT_DIR}

# make kernel binary
bin: prepare ${BOOT_ASM_SOURCES} ${LOADER_ASM_SOURCES}
	nasm ${BOOTSECT_ASM_FLAGS} ${BOOT_ASM_SOURCES} -o ${BOOT_BIN}
	nasm ${BOOTSECT_ASM_FLAGS} ${LOADER_ASM_SOURCES} -o ${LOADER_BIN}

# make build directory
prepare:
	mkdir -p ${BUILD_DIR} ${BUILD_DIR}/${BOOTSECT_DIR} ${MOUNT_DIR}

clean:
	rm -rf build