/* This is the linker script for my kernel supporting UEFI */
ENTRY(kmain_multiboot)
OUTPUT_FORMAT("elf64-x86-64")

SECTIONS 
{
    _KernelStart = .;
    .boot : ALIGN(0x1000)
    {
        _HeaderStart = .;
        *(.multiboot_header)
        _HeaderEnd = .;
    }
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
