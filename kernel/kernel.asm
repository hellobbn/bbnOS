SelectorKernelCS        equ     8

; import function
extern  cstart

; global value
extern  gdt_ptr ; from start.c

; =============================================================
; BSS SECTION
; =============================================================
[SECTION .bss]
StackSpace              resb        2 * 1024
StackTop:               ; top of stack

; =============================================================
; TEXT SECTION
; =============================================================
[SECTION .text]

global _start

_start:
    mov     esp, StackTop

    sgdt    [gdt_ptr]
    call    cstart
    lgdt    [gdt_ptr]

    jmp     SelectorKernelCS:csinit
    
csinit:
    push    0
    popfd

    jmp     $
