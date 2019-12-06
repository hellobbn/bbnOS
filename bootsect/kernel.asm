[SECTION .text]

global _start

_start:
    mov     ax, 0xB800
    mov     gs, ax
    mov     ah, 0x0F
    mov     al, 'K'
    ; it is assumed that gs points to the video memory
    mov     [gs:((80 * 1 + 39) * 2)], ax
    jmp     $