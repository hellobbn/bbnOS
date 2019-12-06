[SECTION .text]

global  outb
global  inb

; outb - Send a byte to an I/O port
; stack: [esp + 8] The data byte
;        [esp + 4] The I/O port
;        [esp]     Return address
outb:
    mov     al, [esp + 8]
    mov     dx, [esp + 4]
    out     dx, al
    ret

; inb - Returns a byte from the given I/O port
; stack: [esp + 4] The address of the I/O port
;        [esp]     Return address
inb:
    mov     dx, [esp + 4]
    in      al, dx              ; return value in eax, in this case: al
    ret

