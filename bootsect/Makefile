BOOT=bootsect/boot
BOOT_OBJS=output/${BOOT}.o
BOOT_BIN=output/${BOOT}.bin

output/${BOOT}.o: ${BOOT}.asm
	nasm ${ASFLAGS} ${BOOT}.asm -o output/${BOOT}.o

output/${BOOT}.bin: ${BOOT_OBJS}
	ld -T bootsect/link16.ld ${BOOT_OBJS} -o output/${BOOT}.elf
	objcopy -O binary output/${BOOT}.elf output/${BOOT}.bin