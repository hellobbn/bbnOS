; asmsyntax=nasm

%include    "./boot.inc"
%include    "./lib.inc"

PageDirBase0            equ         200000h         ; Page Directory Start: 2M, Address
PageTblBase0            equ         201000h         ; Page Table Start: 2M + 4K
PageDirBase1            equ         210000h
PageTblBase1            equ         211000h

LinearAddrDemo          equ         00401000h
ProcFoo                 equ         00401000h
ProcBar                 equ         00501000h
ProcPagingDemo          equ         00301000h

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

    call    DetectMemory

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
    jmp     $
; End of [SECTION .s16]

; -------------------------------------------------------------
; Memory Detection using 0x15 INT
; -------------------------------------------------------------
; note: check osdev for a more accurate thing to check
DetectMemory:
    mov     di, MemChkBuf   ; es:di saves memory information
    xor     ebx, ebx
    mov     dword [MemNum], 0
.do_e820:
    xor     ebx, ebx    ; ebx be 0 at start
    xor     bp, bp
    mov     edx, 0x0534D4150    ; Place "SMAP" into edx
    mov     eax, 0xE820 ; E820!
    mov     [es:di + 20], dword 1   ; force a valid ACPI 3.X entry
    mov     ecx, 24 ; ask for 24 bytes, may include a ACPI 3.x entry
    int     0x15    ; do E820
    jc  short   .failed   ; sorry, failed
    mov     edx, 0x0534D4150    ; some BIOSes trash this register
    cmp     eax, edx    ; eax must be set to "SMAP" if success
    jne short   .failed
    test    ebx, ebx    ; ebx = 0 --> list is only one entry long
    je  short   .failed
    jmp short   .jmpin
.e8201p:
    mov     eax, 0xE820 ; eax, ecx get trashed on each int0x15 call
    mov     [es:di + 20], dword 1   ; again
    mov     ecx, 24
    int     0x15    ; do E820
    jc  short .e820f    ; carry --> end of list already reached
    mov     edx, 0x0534D4150    ; repair potentially trashed register
.jmpin:
    jcxz    .skipent    ; skip any 0 length entries
    cmp     cl, 20  ; check if we get a 24 byte ACPI 3.x response
    jbe     short .notext
    test    byte [es:di + 20], 1    ; if so: is the "ignore this data" bit clear?
    je  short .skipent
.notext:
    mov     ecx, [es:di + 8]    ; get lower uint32_t of memory region length
    or      ecx, [es:di + 12]   ; "or" it with upper uint32_t to test zero
    jz      .skipent    ; if length uint64_t is 0, skip the entry
    inc     dword [MemNum]  ; got a good entry, ++ count
    add     di, 24  ; next
.skipent:
    test    ebx, ebx    ; if ebx->0, list is complete
    jne short   .e8201p ; another E820
.e820f:
    clc
    ret
.failed:
    stc
    ret

; ==============================================================
; 32 BIT SECTION
; ==============================================================
[SECTION .s32]
[BITS    32]

LABEL_SEG_CODE32:
    mov     ax, SelectorData
    mov     ds, ax                                  ; Data Selector
    mov     es, ax
    mov     ax, SelectorVideo
    mov     gs, ax                                  ; Video Selector

    mov     ax, SelectorStack
    mov     ss, ax                                  ; Stack Selector

    mov     esp, TopOfStack

    ; clear the framebuffer here
    call    ClearScreen

    ; entered protected mode, display message
    push    PMMessageOffset
    call    DispStr
    add     esp, 4

    ; print memory message
    push    MemChkTitleOffset
    call    DispStr
    add     esp, 4

    call    PrintMemInfo
    call    DispReturn

    ;call    PagingDemo ; deleted, unable to run.

    ; paging here
    call    SetupPaging

    ; Display a String
    push    osOkMessageOffset
    call    DispStr
    add     esp, 4

    jmp     $

; SetupPaging - set up paging based on current available memory
SetupPaging:
    ; Linear Address equals physical address
    xor     edx, edx
    mov     eax, [dwMemSizeOffset]
    mov     ebx, 400000h    ; one page: 4KB, 1024 pages: 4M
    div     ebx ; eax = eax / ebx, edx = eax % ebx
    mov     ecx, eax
    test    edx, edx    ; do we have reminder?
    jz      .edx_zero
    inc     ecx ; have a reminder, increase ecx
.edx_zero:
    mov     [PageTableNumber], ecx

    ; Initialize page dir
    mov     ax, SelectorFlatRW
    mov     es, ax
    mov     edi, PageDirBase0
    xor     eax, eax
    mov     eax, PageTblBase0 | PG_P | PG_USU | PG_RWW

.1:
    stosd
    add     eax, 4096
    loop    .1

    ; Initialize Page Table
    xor     eax, eax
    mov     ax, [PageTableNumber]                     ; Now for Page Table
    mov     ebx, 1024                        ; 1024 Page Table Entries, 1024 Page Tables
    mul     ebx     ; total: eax * 1024 page tables
    mov     ecx, eax
    mov     edi, PageTblBase0
    xor     eax, eax
    mov     eax, PG_P | PG_USU | PG_RWW             ; Base Address -> 0

.2:
    stosd
    add     eax, 4096
    loop    .2

    mov     eax, PageDirBase0
    mov     cr3, eax                                ; Move PageDirBase to cr3 without modifying any params
    mov     eax, cr0
    or      eax, 80000000h
    mov     cr0, eax                                ; Enable Paging
    jmp     short .3
.3:
    nop

    push    pagingOKMsgOffset
    call    DispStr
    add     esp, 4

    ret
; --------- end of set up paging ---------

; -------------------------------------------------------------
; Print Out Memory Info here
; The Buffer is in MemChkBufOffset
; -------------------------------------------------------------
PrintMemInfo:
    push    esi
    push    edi
    push    ecx

    mov     esi, MemChkBufOffset
    mov     ecx, [MemNumOffset]
.loop:
    mov     edx, 6
    mov     edi, ARDStructOffset
.1:
    push    dword [esi]
    call    DispInt
    pop     eax
    stosd   ; store EAX at [ES:EDI]
    add     esi, 4
    dec     edx
    cmp     edx, 0
    jnz     .1
    call    DispReturn
    cmp     dword [dwTypeOffset], 1
    jne     .2
    xor     eax, eax
    mov     eax, [dwBaseAddrLowOffset]
    add     eax, [dwLengthLowOffset]
    cmp     eax, [dwMemSizeOffset]
    jb      .2
    mov     [dwMemSizeOffset], eax
.2:
    loop    .loop

    call    DispReturn
    push    szRAMSizeOffset
    call    DispStr
    add     esp, 4

    push    dword [dwMemSizeOffset]
    call    DispInt
    add     esp, 4

    pop     ecx
    pop     edi
    pop     esi
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
LABEL_DESC_VIDEO:       Descriptor    0B8000h,             0ffffh,             DA_DRW
LABEL_DESC_FLAT_C:      Descriptor          0,            0fffffh,  DA_CR | DA_32 | DA_LIMIT_4K
LABEL_DESC_FLAT_RW:     Descriptor          0,            0fffffh,  DA_DRW | DA_LIMIT_4K
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
SelectorFlatC       equ     LABEL_DESC_FLAT_C   - LABEL_GDT
SelectorFlatRW      equ     LABEL_DESC_FLAT_RW  - LABEL_GDT
; End of [SECTION .gdt]

; ==============================================================
; Data 1 Section
; ==============================================================
[SECTION .data]
ALIGN   32
[BITS   32]
LABEL_DATA:
SPValueInRealMode       dw      0
; Strings
PMMessage               db      "In Protect Mode Now, setting up..... ^_^", 0Ah, 0
PMMessageOffset         equ     PMMessage - $$

osOkMessage             db      "Done, Welcome to BBN OS!", 0Ah, 0
osOkMessageOffset       equ     osOkMessage - $$

pagingOKMsg             db      "Done Set up paging.", 0Ah, 0
pagingOKMsgOffset       equ     pagingOKMsg - $$

DataLen                 equ     $ - LABEL_DATA

; Memory Check Buffer
MemChkBuf:
    times   512         db      0
MemChkBufOffset         equ     MemChkBuf - $$

MemChkTitle:
    db  "Start Memory Check..........", 0Ah
    db  "BaseAddrL  BaseAddrH LengthLow LengthHigh    Type   ACPI3.x", 0Ah, 0
MemChkTitleOffset       equ     MemChkTitle - $$

ARDStruct:
    dwBaseAddrLow:      dd      0
    dwBaseAddrHigh:     dd      0
    dwLengthLow:        dd      0
    dwLengthHigh:       dd      0
    dwType:             dd      0
    acpi_resp:          dd      0   ; ACPI responce
ARDStructOffset         equ     ARDStruct - $$
    dwBaseAddrLowOffset equ     dwBaseAddrLow - $$
    dwBaseAddrHighOffset equ    dwBaseAddrHigh - $$
    dwLengthLowOffset   equ     dwLengthLow - $$
    dwLengthHighOffset  equ     dwLengthHigh - $$
    dwTypeOffset        equ     dwType - $$
    dwAcpiResp          equ     acpi_resp - $$

dwMemSize:
                        dd      0
dwMemSizeOffset         equ     dwMemSize - $$

szRAMSize:  			db	    "RAM size:", 0
szRAMSizeOffset         equ     szRAMSize - $$

_szReturn:			    db	        0Ah, 0
_dwDispPos:			    dd	        (80 * 1 + 0) * 2	; 屏幕第 1 行, 第 0 列。
szReturn		        equ	        _szReturn	- $$
dwDispPos               equ         _dwDispPos - $$

MemNum:                 dd          0
MemNumOffset            equ         MemNum - $$

_PageTableNumber        dd          0
PageTableNumber         equ         _PageTableNumber - $$
; End of [SECTION .data]

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
