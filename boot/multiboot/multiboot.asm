extern kmain_multiboot

bits 32
section .text

global multiboot_entry
multiboot_entry:
  mov esp, stack_end

  push dword 0
  popf

  mov edi, eax
  mov esi, ebx

  call kmain_multiboot

multiboot_end:
  jmp multiboot_end

section .bss

stack_begin:
  resb 4096 ; reserve 4KiB size
stack_end:
