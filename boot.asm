; asmsyntax=nasm

org     07c00h          ; Tell compiler that this is loaded to 0x7C00

start:
    mov     ax, cs          ; CS -> AX
    mov     ds, ax          ; AX -> DS
    mov     es, ax          ; AX -> ES
    call    DispStr         ; Display String to VGA
    jmp     $               ; Infinity Loop

; =================================================
; DispStr: Calling INT 0x10 to Display Hello World
; =================================================
; INT 10H: 
;       - AH = 13H: Write String, 
;       - AL = Write Mode 
;       - BH = Page Number
;       - BL = Color
;       - CX = String Length
;       - DH = Row
;       - DL = Column
;       - ES:BP = Offset of String

DispStr:
    mov     ax, BootMessage
    mov     bp, ax          ; ES:BP: Offset of String
    mov     cx, 16          ; String Length
    mov     ax, 01301h      ; AH = 0x13, AL = 0x01
    mov     bx, 0000ch      ; BH = 0x00, BL = 0x0C
    mov     dl, 0           
    int     10h             ; Interrupt 10
    ret

BootMessage:            db      "Hello, OS world!"
times   510 - ($ - $$)  db      0                   ; Fill 0 for rest of the space
dw      0xAA55              ; End Sign, BootSectore must start with this
