####################################################
# Makefile for bbnOS Following the guide of OrangeS#
####################################################

## OS Detection
UNAME := $(shell uname)

## UEFI Flag
UEFI_KERNEL ?= true

## Multiboot flag, if this is true, the kernel's built-in loader will
## not be built
MULTIBOOT_KERNEL ?= true

## DIRs
BUILD_DIR		= build
MOUNT_POINT		= ${BUILD_DIR}/mnt

ifeq (${MULTIBOOT_KERNEL}, false)
ifeq (${UEFI_KERNEL}, false)
BOOT_DIR		= boot/legacy
else
BOOT_DIR    = boot/uefi
endif
else
BOOT_DIR    = boot/multiboot
endif

ifeq (${UEFI_KERNEL}, false)
KERNEL_DIR  = kernel/legacy
INC_K_DIR		= include/legacy
INC_B_DIR   = boot/legacy/include
LIB_DIR			= lib/legacy
else
KERNEL_DIR  = kernel/uefi
INC_K_DIR		= include/uefi
LIB_DIR     = lib/uefi
endif
SCRIPT_DIR		= scripts
RESOURCE_DIR  = resources

DIRS 			= ${BOOT_DIR} ${KERNEL_DIR} ${LIB_DIR}
MKDIR			= ${patsubst %, ${BUILD_DIR}/%, ${DIRS}}
MKDIR		  += ${BUILD_DIR}
MKDIR     += ${MOUNT_POINT}

## Programs, flags, etc
ASM				= nasm
CC				= clang
CXX 			= clang++
LD				= ld.lld
OBJCOPY   = objcopy
ASM_B_FLAGS		= -I ${INC_B_DIR}
ASM_K_FLAGS		= -I ${INC_K_DIR}
C_FLAGS			= -c -I ${INC_K_DIR} -fno-builtin -Wall -Wextra -fno-stack-protector -g

ifeq ($(UEFI_KERNEL), false)
LDFLAGS			= -T ${SCRIPT_DIR}/link.ld
else ifeq (${MULTIBOOT_KERNEL}, true)
LDFLAGS			= -T ${SCRIPT_DIR}/multiboot.ld
else
LDFLAGS 		= -T ${SCRIPT_DIR}/uefi.ld -static -Bsymbolic -nostdlib
endif

# Add macro if it is a UEFI kernel
ifeq ($(UEFI_KERNEL), true)
C_FLAGS += -DUEFI_KERNEL
ASM_K_FLAGS += -f elf64
else
C_FLAGS += -m32
ASM_K_FLAGS += -f elf32
endif

# Add macro if it is a multiboot kernel
C_FLAGS += -DMULTIBOOT_KERNEL

C_FLAGS += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function

CXX_FLAGS = ${C_FLAGS}

## This Program
ifeq (${MULTIBOOT_KERNEL}, false)
ifeq (${UEFI_KERNEL}, false)
# For boot
BOOT_ASM		= ${BOOT_DIR}/boot.asm
BOOT        = ${BUILD_DIR}/${BOOT_DIR}/boot.bin

LOADER_ASM		= ${BOOT_DIR}/loader.asm
LOADER      	= ${BUILD_DIR}/${BOOT_DIR}/loader.bin
else
UEFI_BOOT_IMG = ${BUILD_DIR}/loader.efi

GNUEFI_DIR    = ./boot/uefi/gnu-efi
GNUEFI_BUILD_DIR = ${GNUEFI_DIR}/x86_64/bootloader
UEFI_LOADER = ${GNUEFI_BUILD_DIR}/uefi_main.efi
endif
endif

# For kernel
KERN_ASMS		= ${wildcard ${KERNEL_DIR}/*.asm}
ifeq (${MULTIBOOT_KERNEL}, true)
KERN_ASMS   += ${wildcard ${BOOT_DIR}/*.asm}
endif
KERN_ASM_OBJS	= ${patsubst %.asm, ${BUILD_DIR}/%.obj, ${KERN_ASMS}}
KERN_C			= ${wildcard ${KERNEL_DIR}/*.c}
ifeq (${MULTIBOOT_KERNEL}, true)
KERN_C   += ${wildcard ${BOOT_DIR}/*.c}
endif
KERN_C_OBJS	    = ${patsubst %.c, ${BUILD_DIR}/%.o, ${KERN_C}}
KERN_CXX    = ${wildcard ${KERNEL_DIR}/*.cpp}
KERN_CXX_OBJS   = ${patsubst %.cpp, ${BUILD_DIR}/%.obj, ${KERN_CPP}}

# For lib
ifeq (${UEFI_KERNEL}, false)
LIB_ASMS		= ${wildcard ${LIB_DIR}/*.asm}
KERN_ASM_OBJS  += ${patsubst %.asm, ${BUILD_DIR}/%.obj, ${LIB_ASMS}}
endif
LIB_C			= ${wildcard ${LIB_DIR}/*.c}
KERN_C_OBJS    += ${patsubst %.c, ${BUILD_DIR}/%.o, ${LIB_C}}
LIB_CXX   = ${wildcard ${LIB_DIR}/*.cpp}
KERN_CXX_OBJS  += ${patsubst %.cpp, ${BUILD_DIR}/%.obj, ${LIB_CXX}}

# For the out kernel
ifeq ($(UEFI_KERNEL), false)
KERNEL 			= ${BUILD_DIR}/kernel.bin
else
KERNEL      = ${BUILD_DIR}/kernel.elf
endif

## The output image
IMG_OUT			= ${BUILD_DIR}/os.img

## Rewrite something for MacOS
ifeq ($(UNAME), Darwin)
CC 				= i386-elf-gcc
LD				= i386-elf-ld
endif

ifeq ($(MULTIBOOT_KERNEL), true)
	GRUB_IMG_DIR = ${BUILD_DIR}/grub_img
	KERNEL       = ${BUILD_DIR}/kernel.bin
endif

## rules

# all
all: img

# (Optional QEMU)
ifeq (${UEFI_KERNEL}, false)
qemu: img
	qemu-system-i386 -fda ${IMG_OUT}

qemu_gdb: img
	qemu-system-i386 -fda ${IMG_OUT} -s -S
else
qemu: img
	@echo
	@echo "==> Starting qemu"
	qemu-system-x86_64 -drive file=${IMG_OUT} \
		-drive if=pflash,format=raw,unit=0,file=./resources/OVMF_CODE.fd \
		-drive if=pflash,format=raw,unit=1,file=./resources/OVMF_VARS.fd \
		-m 256M -cpu qemu64

qemu_gdb: img
	@echo
	@echo "==> Starting qemu"
	qemu-system-x86_64 -drive file=${IMG_OUT} \
		-drive if=pflash,format=raw,unit=0,file=./resources/OVMF_CODE.fd \
		-drive if=pflash,format=raw,unit=1,file=./resources/OVMF_VARS.fd \
		-m 256M -cpu qemu64 -s -S
endif

# build the image
img: all_aval_img
	@echo
	@echo "==> Building bootimage"
ifeq (${MULTIBOOT_KERNEL}, false)
ifeq ($(UEFI_KERNEL), false)
	dd if=/dev/zero of=${IMG_OUT} bs=512 count=2880
	dd if=${BOOT} of=${IMG_OUT} bs=512 count=1 conv=notrunc
ifeq ($(UNAME), Linux)
	sudo mount -o loop ${IMG_OUT} ${MOUNT_POINT}
else
	hdiutil mount ${IMG_OUT} -mountpoint ${MOUNT_POINT}
endif
	sudo cp ${LOADER} ${MOUNT_POINT}
	sudo cp ${KERNEL} ${MOUNT_POINT}
ifeq ($(UNAME), Linux)
	sudo umount ${MOUNT_POINT}
else
	hdiutil unmount ${MOUNT_POINT}
endif
else
	dd if=/dev/zero of=${IMG_OUT} bs=512 count=93750
	mkfs.vfat ${IMG_OUT}
	mmd -i ${IMG_OUT} ::/EFI
	mmd -i ${IMG_OUT} ::/EFI/BOOT
	mcopy -i ${IMG_OUT} ${UEFI_BOOT_IMG} ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i ${IMG_OUT} ${KERNEL} ::
# Move the font to img
	mcopy -i ${IMG_OUT} ${RESOURCE_DIR}/default_font.psf ::
endif
else
	mkdir -p ${GRUB_IMG_DIR}/boot/grub
	cp ${SCRIPT_DIR}/grub.cfg ${GRUB_IMG_DIR}/boot/grub
	cp ${KERNEL} ${GRUB_IMG_DIR}/boot
	grub-mkrescue -o ${BUILD_DIR}/os.img ${GRUB_IMG_DIR}
endif

ifeq (${MULTIBOOT_KERNEL}, false)
ifeq ($(UEFI_KERNEL), false)
all_aval_img: prepare ${BOOT} ${LOADER} ${KERNEL}
else
all_aval_img: prepare gnuefi ${UEFI_BOOT_IMG} ${KERNEL}
endif
else
all_aval_img: prepare ${KERNEL}
endif

ifeq (${UEFI_KERNEL}, false)
${BOOT}: ${BUILD_DIR}/%.bin: %.asm
	${ASM} ${ASM_B_FLAGS} -o $@ $<

${LOADER}: ${BUILD_DIR}/%.bin: %.asm
	${ASM} ${ASM_B_FLAGS} -o $@ $<
else
${UEFI_BOOT_IMG}:
	@echo
	@echo "==> Building Boot Loader"
	make -C ${GNUEFI_DIR} bootloader
	cp ${UEFI_LOADER} ${UEFI_BOOT_IMG}
endif


# FIXME: The linking sequence is important
#        To be specific, the kernel.o compiled from kernel.asm must be compiled first
ifeq ($(UEFI_KERNEL), false)
${KERNEL}: pre_build_kernel ${KERN_C_OBJS} ${KERN_ASM_OBJS}
else
${KERNEL}: pre_build_kernel ${KERN_C_OBJS} ${KERN_CXX_OBJS} ${KERN_ASM_OBJS}
endif
		${LD} ${LDFLAGS} -o ${KERNEL} ${KERN_ASM_OBJS} ${KERN_C_OBJS} ${KERN_CXX_OBJS}

${KERN_C_OBJS}: ${BUILD_DIR}/%.o: %.c
	${CC} ${C_FLAGS} -o $@ $<

${KERN_CXX_OBJS}: ${BUILD_DIR}/%.obj: %.cpp
	${CXX} ${CXX_FLAGS} -o $@ $<

${KERN_ASM_OBJS}: ${BUILD_DIR}/%.obj: %.asm
	${ASM} ${ASM_K_FLAGS} -o $@ $<

pre_build_kernel:
	@echo
	@echo "==> Building Kernel"

# make necessary dirs
prepare:
	mkdir -p ${MKDIR}
ifeq (${UEFI_KERNEL}, false)
	@echo "==============================="
	@echo " Building for Legacy Kernel    "
	@echo "==============================="
else
	@echo "==============================="
	@echo " Building for UEFI Kernel    "
	@echo "==============================="
endif

gnuefi:
	@echo "==> Silencing make gnuefi..."
	make -C ${GNUEFI_DIR} -j$(nproc) > /dev/null

# clean
clean:
	@echo "==> Clean All"
	rm -rf ${BUILD_DIR}
ifeq (${MULTIBOOT_KERNEL}, false)
ifeq (${UEFI_KERNEL}, true)
	make -C ${GNUEFI_DIR} clean > /dev/null
endif
endif
