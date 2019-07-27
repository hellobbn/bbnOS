/* 
 * link16.ld
 * 
 * Linker script for the i386 bootsect code 
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS
{
        . = 0x0100;
        .s16         : { *(.s16) }
        .s32         : { *(.s32) }
        .gdt         : { *(.gdt) }
        .data1       : { *(.data1) }
        .gs          : { *(.gs) }
        .dat         : { *(.dat) }
        .text        : { *(.text)}

/*        . = 510 + 0x0100; */
/*        .header      : { *(.header) } */
}
