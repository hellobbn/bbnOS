_NR_get_ticks   equ     0
INT_VECTOR_SYS_CALL equ 0x90

global test_call

bits 32
[section .text]

test_call:
    mov     eax, _NR_get_ticks
    int     INT_VECTOR_SYS_CALL
    ret