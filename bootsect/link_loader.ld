/* 
 * link_loader.ld
 * 
 * Linker script for the i386 loader
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS
{
        . = 0x0100;
        .text         : { *(.text) }
}