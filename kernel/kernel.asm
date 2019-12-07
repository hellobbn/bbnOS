SelectorKernelCS        equ     8

; import function
extern  k_start_msg
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
    call    k_start_msg ; print kernel message
    mov     esp, StackTop

    sgdt    [gdt_ptr]   ; save the loader's GDT ptr to the given point
    call    cstart      ; copy the content of GDT
    lgdt    [gdt_ptr]   ; reload GDT ptr

    jmp     SelectorKernelCS:csinit
    
csinit:
    push    0
    popfd

    jmp     $
