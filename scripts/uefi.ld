/* This is the linker script for my kernel supporting UEFI */
ENTRY(kmain)
OUTPUT_FORMAT("elf64-x86-64")

SECTIONS 
{
    _KernelStart = .;
    .text : ALIGN(0x1000)
    {
        *(.text)
    }
    .data : ALIGN(0x1000)
    {
        *(.data)
    }
    .rodata : ALIGN(0x1000)
    {
        *(.rodata)
    }
    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
    }
    _KernelEnd = .;
}
