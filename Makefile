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
LOADER_ASM_FLAGS = -f elf32 -Ibootsect
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
	qemu-system-i386 -cdrom ${ISO_OUT}

qemu_gdb: iso
	qemu-system-i386 -cdrom ${ISO_OUT} -s -S

qemu_nogra: iso
	qemu-system-i386 -cdrom ${ISO_OUT} -nographic

qemu_nogra_gdb: iso
	qemu-system-i386 -cdrom ${ISO_OUT} -nographic -s -S

# iso
iso: bin ${MOUNT_DIR}
	dd if=/dev/zero of=${OUT_IMG} bs=512 count=2880
	dd if=${BOOT_BIN} of=${OUT_IMG} bs=512 count=1 conv=notrunc
	sudo mount -o loop ${OUT_IMG} ${MOUNT_DIR}
	sudo cp ${LOADER_BIN} ${MOUNT_DIR} -v
	sudo umount ${MOUNT_DIR}

# make kernel binary
bin: prepare ${BOOT_OBJS} ${LOADER_OBJS}
	objcopy -O binary ${BOOT_OBJS} ${BOOT_BIN}
	objcopy -O binary ${LOADER_OBJS} ${LOADER_BIN}

# linking here
${BOOT_OBJS}: ${BOOT_ASM_OBJS}
	${LD} ${LD_FLAGS} -T bootsect/link16.ld $^ -o $@ 

${LOADER_OBJS}: ${LOADER_ASM_OBJS}
	${LD} ${LD_FLAGS} -T bootsect/link_loader.ld $^ -o $@

# compile .asm files to .o files
${BOOT_ASM_OBJS}: ${BUILD_DIR}/${BOOTSECT_DIR}/%.obj: ${BOOTSECT_DIR}/%.asm
	${ASM} ${LOADER_ASM_FLAGS} -o $@ $<

${LOADER_ASM_OBJS}: ${BUILD_DIR}/${BOOTSECT_DIR}/%.obj: ${BOOTSECT_DIR}/%.asm
	${ASM} ${LOADER_ASM_FLAGS} -o $@ $<

# make build directory
prepare:
	mkdir -p ${BUILD_DIR} ${BUILD_DIR}/${BOOTSECT_DIR} ${MOUNT_DIR}

clean:
	rm -rf build