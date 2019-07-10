; asmsyntax=nasm

%include    "boot.inc"

jmp     LABEL_START

[SECTION    .s16]
[BITS       16]
LABEL_START:
    mov     ax, cs                              ; CS -> AX
    mov     ds, ax                              ; AX -> DS
    mov     es, ax                              ; AX -> ES
    mov     ss, ax                              ; AX -> SS
    mov     sp, 0100h
    ;call    DispStr                             ; Display "Hello, World"

    ; Initialize 32-bit Segment Descriptor
    xor     eax, eax                            ; 0 -> EAX
    mov     ax, cs                              ; CS -> AX
    shl     eax, 4                              ; EAX << 4
    add     eax, LABEL_SEG_CODE32
    mov     word [LABEL_DESC_CODE32 + 2], ax    ; AX -> [LAB...] 
    shr     eax, 16
    mov     byte [LABEL_DESC_CODE32 + 4], al
    mov     byte [LABEL_DESC_CODE32 + 7], ah

    ; Prepare for loading GDTR
    xor     eax, eax
    mov     ax, ds
    shl     eax, 4
    add     eax, LABEL_GDT                      ; EAX <- GDT Base Address
    mov     dword [GDTPtr + 2], eax             ; [GDTPtr + 2] <- GDT Base Address

    ; Load GDTR
    lgdt    [GDTPtr]

    ; Disable Interrupt
    cli

    ; Open Address Line A20
    in      al, 92h
    or      al, 00000010b
    out     92h, al

    ; Prepare to switch to Protected Mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; Jump INTO Protect Mode
    jmp     SelectorCode32:0                    ; This will load SelectorCode32 into CS and jump

    ; Never HERE
    jmp     $                                   ; Infinity Loop

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
    mov     bp, ax                                  ; ES:BP: Offset of String
    mov     cx, 16                                  ; String Length
    mov     ax, 01301h                              ; AH = 0x13, AL = 0x01
    mov     bx, 0000ch                              ; BH = 0x00, BL = 0x0C
    mov     dl, 0           
    int     10h                                     ; Interrupt 10
    ret

; End of [SECTION .s16]

[SECTION .s32]
[BITS    32]

LABEL_SEG_CODE32:
    mov     ax, SelectorVideo
    mov     gs, ax

    mov     edi, (80 * 11 + 79) * 2                 ; Screen Column 79, Row 11
    mov     ah, 0Ch                                 ; 0000: Black Background; 1100: Red Text
    mov     al, 'P'
    mov     [gs:edi], ax

    ; Stop HERE
    jmp     $

SegCode32Len        equ         $ - LABEL_SEG_CODE32

[SECTION .gdt]
; GDT
;                                        Base               Limit               Attr
LABEL_GDT:              Descriptor          0,                  0,                  0
LABEL_DESC_CODE32:      Descriptor          0,   SegCode32Len - 1,       DA_C + DA_32
LABEL_DESC_VIDEO:       Descriptor    0B8000h,             0ffffh,             DA_DRW         
; End of GDT

GDTLen              equ     $-LABEL_GDT         ; Length of GDT
GDTPtr              dw      GDTLen - 1          ; GDT boundary, 2 bytes
                    dd      0                   ; GDT Base Address, 4 bytes
; GDT Selector
SelectorCode32      equ     LABEL_DESC_CODE32 - LABEL_GDT       ; 8
SelectorVideo       equ     LABEL_DESC_VIDEO  - LABEL_GDT       ; 16

; End of [SECTION .gdt]

; Data Session
[SECTION .dat]
BootMessage:            db      "Hello, OS world!"

[SECTION .header]
dw      0xAA55                                      ; End Sign, BootSectore must start with this