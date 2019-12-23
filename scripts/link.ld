/* This is the linker script for my kernel */
ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS 
{
    . = 0x30400;
    .text ALIGN(0x1000) : {
        build/kernel/kernel.obj(.text)  /* This must be linked first */
        *(.text)
    }

    .data ALIGN(0x1000) : {
        *(.data)
    }

    .bss ALIGN(0x1000) : {
        *(.bss)
    }
}