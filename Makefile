# source code folders
BUILD_DIR 			= 		build
BOOTSECT_DIR 		= 		bootsect

# bootsect for floppy
BOOT_ASM_SOURCES 	= 		${BOOTSECT_DIR}/boot.asm
BOOT_BIN 			= 		${BUILD_DIR}/boot.bin		# boot binary

# loader for kernel
LOADER_ASM_SOURCES 	= ${BOOTSECT_DIR}/loader.asm
LOADER_BIN 			= ${BUILD_DIR}/loader.bin	# the loader

# the kernel
KERNEL_ASM_SOURCES 	= ${BOOTSECT_DIR}/kernel.asm
KERNEL_ASM_OBJS		= ${BOOTSECT_DIR}/kernel.obj
KERNEL_BIN			= ${BUILD_DIR}/kernel.bin

# the output iso
OUT_IMG 			= ${BUILD_DIR}/os.img
MOUNT_DIR 			= ${BUILD_DIR}/mnt

# build tools
# ASM compiler
ASM 				= nasm
KERNEL_ASM_FLAGS 	= -f elf32
BOOTSECT_ASM_FLAGS 	= -Ibootsect

# C compiler
CC = gcc
CC_FLAGS = -c -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
-Wall -Wextra

# ther linker
LD = ld
KERNEL_LD_FLAGS = -s -m elf_i386


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
	sudo cp ${KERNEL_BIN} ${MOUNT_DIR} -v
	sudo umount ${MOUNT_DIR}

# make kernel binary
bin: prepare ${BOOT_ASM_SOURCES} ${LOADER_ASM_SOURCES} ${KERNEL_BIN}
	nasm ${BOOTSECT_ASM_FLAGS} ${BOOT_ASM_SOURCES} -o ${BOOT_BIN}
	nasm ${BOOTSECT_ASM_FLAGS} ${LOADER_ASM_SOURCES} -o ${LOADER_BIN}

# the kernel binary
${KERNEL_BIN}: ${KERNEL_ASM_SOURCES}
	${ASM} ${KERNEL_ASM_FLAGS} -o ${KERNEL_ASM_OBJS} ${KERNEL_ASM_SOURCES}
	${LD} ${KERNEL_LD_FLAGS} -o ${KERNEL_BIN} ${KERNEL_ASM_OBJS}

# make build directory
prepare:
	mkdir -p ${BUILD_DIR} ${BUILD_DIR}/${BOOTSECT_DIR} ${MOUNT_DIR}

clean:
	rm -rf build