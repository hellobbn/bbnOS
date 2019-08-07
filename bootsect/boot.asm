; asmsyntax=nasm

%include    "./boot.inc"

PageDirBase             equ         200000h         ; Page Directory Start: 2M
PageTblBase             equ         201000h         ; Page Table Start: 2M + 4K

jmp     LABEL_START

; ==============================================================
; 16 BIT Section
; ==============================================================
[SECTION    .s16]
[BITS       16]
LABEL_START:
    mov     ax, cs                              ; CS -> AX
    mov     ds, ax                              ; AX -> DS
    mov     es, ax                              ; AX -> ES
    mov     ss, ax                              ; AX -> SS
    mov     sp, 0100h

    mov     [LABEL_GO_BACK_TO_REAL + 3], ax
    mov     [SPValueInRealMode], sp

    ; Initialize 16-bit Segment Descriptor
    xor     eax, eax
    mov     ax, cs                              ; Move with Zero-Extend
    shl     eax, 4
    add     eax, LABEL_SEG_CODE16
    mov     word [LABEL_DESC_CODE16 + 2], ax
    shr     eax, 16
    mov     byte [LABEL_DESC_CODE16 + 4], al
    mov     byte [LABEL_DESC_CODE16 + 7], ah

    ; Initialize 32-bit Segment Descriptor
    xor     eax, eax                            ; 0 -> EAX
    mov     ax, cs                              ; CS -> AX
    shl     eax, 4                              ; EAX << 4
    add     eax, LABEL_SEG_CODE32
    mov     word [LABEL_DESC_CODE32 + 2], ax    ; AX -> [LAB...]
    shr     eax, 16
    mov     byte [LABEL_DESC_CODE32 + 4], al
    mov     byte [LABEL_DESC_CODE32 + 7], ah

    ; Initialize Data Segment Descriptor
    xor     eax, eax
    mov     ax, ds
    shl     eax, 4
    add     eax, LABEL_DATA
    mov     word [LABEL_DESC_DATA + 2], ax
    shr     eax, 16
    mov     byte [LABEL_DESC_DATA + 4], al
    mov     byte [LABEL_DESC_DATA + 7], ah

    ; Initialize Stack Descriptor
    xor     eax, eax
    mov     ax, ds
    shl     eax, 4
    add     eax, LABEL_STACK
    mov     word [LABEL_DESC_STACK + 2], ax
    shr     eax, 16
    mov     byte [LABEL_DESC_STACK + 4], al
    mov     byte [LABEL_DESC_STACK + 7], ah

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
    jmp     dword SelectorCode32:0              ; This will load SelectorCode32 into CS and jump

    ; Never HERE
    jmp     $                                   ; Infinity Loop

; ------------------------------------------------
; DispStr: Calling INT 0x10 to Display Hello World
; ------------------------------------------------
; INT 10H:
;       - AH = 13H: Write String,
;       - AL = Write Mode
;       - BH = Page Number
;       - BL = Color
;       - CX = String Length
;       - DH = Row
;       - DL = Column
;       - ES:BP = Offset of String

; DispStr:
;     mov     ax, BootMessage
;     mov     bp, ax                                  ; ES:BP: Offset of String
;     mov     cx, 16                                  ; String Length
;     mov     ax, 01301h                              ; AH = 0x13, AL = 0x01
;     mov     bx, 0000ch                              ; BH = 0x00, BL = 0x0C
;     mov     dl, 0
;     int     10h                                     ; Interrupt 10
;     ret

LABEL_REAL_ENTRY:
    mov     ax, cs                                  ; Recover all segment registers
    mov     ds, ax
    mov     es, ax
    mov     ss, ax

    mov     sp, [SPValueInRealMode]                 ; Recover SP
    in      al, 92h
    and     al, 11111101b
    out     92h, al                                 ; Address Line A20

    sti                                             ; Enable Interrupt

    xor     eax, eax
    mov     ax, 4c00h
    int     21h                                     ; Return to DOS
; End of [SECTION .s16]


; ==============================================================
; 32 BIT SECTION
; ==============================================================
[SECTION .s32]
[BITS    32]

LABEL_SEG_CODE32:
    call    SetupPaging

    mov     ax, SelectorData
    mov     ds, ax                                  ; Data Selector
    mov     ax, SelectorVideo
    mov     gs, ax                                  ; Video Selector

    mov     ax, SelectorStack
    mov     ss, ax                                  ; Stack Selector

    mov     esp, TopOfStack

    ; Display a String
    mov     ah, 0Ch                                 ; Black; Red
    xor     esi, esi                                ; ESI = 0
    xor     edi, edi                                ; EDI = 0
    mov     esi, OffsetPMMessage                    ; Message Offset
    mov     edi, (80 * 10 + 0) * 2                  ; Row 10, Colomn 0
    cld                                             ; Clear DF Flag, ESI inc
.1:
    lodsb                                           ; Load byte at address DS:(E)SI into AL
    test    al, al
    jz      .2
    mov     [gs:edi],   ax
    add     edi, 2
    jmp     .1
.2:         ; End of Display

    jmp     SelectorCode16:0

SetupPaging:
    ; Linear Address equals physical address

    ; Initialize Page Directory
    mov     ax, SelectorPageDir
    mov     es, ax
    mov     ecx, 1024
    xor     edi, edi
    xor     eax, eax
    mov     eax, PageTblBase | PG_P | PG_USU | PG_RWW

.1:
    stosd
    add     eax, 4096
    loop    .1

    ; Initialize Page Table
    mov     ax, SelectorPageTbl
    mov     es, ax
    mov     ecx, 1024 * 1024
    xor     edi, edi
    xor     eax, eax
    mov     eax, PG_P | PG_USU | PG_RWW

.2:
    stosd
    add     eax, 4096
    loop    .2

    mov     eax, PageDirBase
    mov     cr3, eax
    mov     eax, cr0
    or      eax, 80000000h
    mov     cr0, eax
    jmp     short .3
.3:
    nop

    ret
; ----------------------------------------------------
SegCode32Len        equ         $ - LABEL_SEG_CODE32

; ==============================================================
; GDT
; ==============================================================
[SECTION .gdt]
; GDT
;                                        Base               Limit               Attr
LABEL_GDT:              Descriptor          0,                  0,                  0
LABEL_DESC_NORMAL:      Descriptor          0,             0ffffh,             DA_DRW
LABEL_DESC_CODE32:      Descriptor          0,   SegCode32Len - 1,       DA_C + DA_32
LABEL_DESC_CODE16:      Descriptor          0,             0ffffh,               DA_C
LABEL_DESC_DATA:        Descriptor          0,        DataLen - 1,             DA_DRW
LABEL_DESC_STACK:       Descriptor          0,         TopOfStack,    DA_DRWA + DA_32
LABEL_DESC_VIDEO:       Descriptor    0B8000h,             0ffffh,             DA_DRW + DA_DPL3
LABEL_DESC_PAGE_DIR:    Descriptor PageDirBase,              4095,             DA_DRW
LABEL_DESC_PAGE_TBL:    Descriptor PageTblBase,              1023,             DA_DRW | DA_LIMIT_4K
; End of GDT

; Gate
; End of Gate

GDTLen              equ     $-LABEL_GDT         ; Length of GDT
GDTPtr              dw      GDTLen - 1          ; GDT boundary, 2 bytes
                    dd      0                   ; GDT Base Address, 4 bytes
; GDT Selector
SelectorNormal      equ     LABEL_DESC_NORMAL   - LABEL_GDT
SelectorCode32      equ     LABEL_DESC_CODE32   - LABEL_GDT
SelectorCode16      equ     LABEL_DESC_CODE16   - LABEL_GDT
SelectorData        equ     LABEL_DESC_DATA     - LABEL_GDT
SelectorStack       equ     LABEL_DESC_STACK    - LABEL_GDT
SelectorVideo       equ     LABEL_DESC_VIDEO    - LABEL_GDT
SelectorPageDir     equ     LABEL_DESC_PAGE_DIR - LABEL_GDT
SelectorPageTbl     equ     LABEL_DESC_PAGE_TBL - LABEL_GDT
; End of [SECTION .gdt]

; ==============================================================
; Data 1 Section
; ==============================================================
[SECTION .data1]
ALIGN   32
[BITS   32]
LABEL_DATA:
SPValueInRealMode       dw      0
; Strings
PMMessage               db      "In Protect Mode Now, with PAGING!!! ^_^", 0
OffsetPMMessage         equ     PMMessage - $$
DataLen                 equ     $ - LABEL_DATA
; End of [SECTION .data1]

; ==============================================================
; Global Stack
; ==============================================================
[SECTION .gs]
ALIGN 32
[BITS   32]
LABEL_STACK:
    times   512         db      0

TopOfStack              equ     $ - LABEL_STACK - 1
; End of [SECTION .gs]

; ==============================================================
; From Real Mode to Protect Mode
; ==============================================================
[SECTION .s16code]
ALIGN   32
[BITS   16]
LABEL_SEG_CODE16:
    ; Jump to Real Mode
    mov     ax, SelectorNormal
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    mov     eax, cr0
    and     al, 11111110b
    mov     cr0, eax

LABEL_GO_BACK_TO_REAL:
    jmp     0:LABEL_REAL_ENTRY

Code16Len               equ     $ - LABEL_SEG_CODE16