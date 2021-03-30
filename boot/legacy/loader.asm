; The loader
org 0x0100

BaseOfStack         equ     0x0100
PageDirBase         equ     100000H
PageTblBase         equ     101000H

jmp     LABEL_START

; for FAT12 header
%include "fat12hdr.inc"

; for GDT macro
%include "pm.inc"

; for some address
%include "load.inc"

; =============================================================
; GDT here 
; =============================================================
;                               Base        Limit   Attribute
LABEL_GDT:          Descriptor       0,          0,      0       ; empty
LABEL_DESC_FLAT_C:  Descriptor       0,    0FFFFFH, DA_CR | DA_32 | DA_LIMIT_4K  ; 0 - 4G
LABEL_DESC_FLAT_RW: Descriptor       0,    0FFFFFH, DA_DRW | DA_32 | DA_LIMIT_4K ; 0 - 4G
LABEL_DESC_VIDEO:   Descriptor 0B8000H,     0FFFFH, DA_DRW | DA_DPL3    ; video memory

GdtLen      equ     $ - LABEL_GDT
GdtPtr      dw      GdtLen - 1      ; Limit
            dd      BaseOfLoaderPhyAddr + LABEL_GDT

; GDT Selector
SelectorFlatC       equ     LABEL_DESC_FLAT_C   - LABEL_GDT
SelectorFlatRW      equ     LABEL_DESC_FLAT_RW  - LABEL_GDT
SelectorVideo       equ     LABEL_DESC_VIDEO    - LABEL_GDT     + SA_RPL3

LABEL_START:
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, BaseOfStack

    mov     dh, 0
    call    DispStr

    ; Check memory here
    call    DetectMemory

    ; search for KERNEL.BIN
    mov     word [wSectorNo], SectorNoOfRootDirectory   ; root directory starts at 19
    xor     ah, ah
    xor     dl, dl
    int     0x13

LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
    cmp     word [wRootDirSizeForLoop], 0   ; chack if root is read over
    jz      LABEL_NO_KERNEL
    dec     word [wRootDirSizeForLoop]
    mov     ax, BaseOfKernelFile
    mov     es, ax  ; es <- Base of loader
    mov     bx, OffsetOfKernelFile  ; bx is now the offset of loader
    mov     ax, [wSectorNo] ; ax <- some sector in root
    mov     cl, 1
    call    ReadSector  ; data -> es:bx

    mov     si, KernelFileName  ; ds:si <- "LOADER  BIN"
    mov     di, OffsetOfKernelFile  ; es:di <- BaseOfLoader:0100
    cld
    mov     dx, 0x10    ; each entry: 0x20 long

LABEL_SEARCH_FOR_KERNEL:
    cmp     dx, 0
    jz      LABEL_NEXT_SEARCH   ; not in this sector, next
    dec     dx
    mov     cx, 11

LABEL_CMP_FILENAME:
    cmp     cx, 0
    jz      LABEL_KERNEL_FOUND  ; found loader
    dec     cx
    lodsb   ; al <- ds:si
    cmp     al, byte [es:di]
    jz      LABEL_GO_ON
    jmp     LABEL_DIFFERENT ; different name

LABEL_GO_ON:
    inc     di
    jmp     LABEL_CMP_FILENAME

LABEL_DIFFERENT:
    and     di, 0xFFE0  ; di = di & 0xE0, reset to start of the thing
    add     di, 20h
    mov     si, KernelFileName
    jmp     LABEL_SEARCH_FOR_KERNEL

LABEL_NEXT_SEARCH:
    add     word [wSectorNo], 1
    jmp     LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_KERNEL:
    mov     dh, 2   ; "no kernel"
    call    DispStr
    jmp     $   ; sad

LABEL_KERNEL_FOUND:
    mov     ax, RootDirSectors
    and     di, 0xFFE0  ; reset to the start of the entry

    push    eax
    mov     eax, [es:di + 0x1C] ; ELF file, size of the kernel
    mov     dword [dwKernelSize], eax   ; save the size of kernel.bin

    pop     eax
    add     di, 0x1A    ; the first cluster saved here
    mov     cx, word [es:di]    ; move it to cx
    push    cx
    add     cx, ax  ; the real sector: X + RootDirSectors + 19 - 2
    add     cx, DeltaSectorNo   ; cx is now the actual sector
    mov     ax, BaseOfKernelFile
    mov     es, ax
    mov     bx, OffsetOfKernelFile
    mov     ax, cx  ; save cx to ax

LABEL_GOON_LOADING_FILE:
    push    ax
    push    bx
    mov     ah, 0x0E
    mov     al, '.'     ; for each sector, print a '.'
    mov     bl, 0x0F
    int     0x10
    pop     bx
    pop     ax

    mov     cl, 1
    call    ReadSector  ; read the sector
    pop     ax
    call    GetFATEntry
    cmp     ax, 0x0FFF
    jz      LABEL_FILE_LOADED
    push    ax
    mov     dx, RootDirSectors
    add     ax, dx
    add     ax, DeltaSectorNo
    add     bx, [BPB_BytesPerSec]
    jmp     LABEL_GOON_LOADING_FILE

LABEL_FILE_LOADED:
    call    KillMotor

    mov     dh, 1       ; "Ready."
    call    DispStr

    lgdt    [GdtPtr]

    cli

    ; A20
    in      al, 0x92
    or      al, 00000010b
    out     0x92, al

    ; protect mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; the base is 0, so use physical address
    jmp     dword SelectorFlatC:(BaseOfLoaderPhyAddr + LABEL_PM_START)
    jmp $

; -------------------------------------------------------------
; Some variable
; -------------------------------------------------------------
wRootDirSizeForLoop     dw  RootDirSectors  ; number of sector in Root Directory
wSectorNo               dw  0               ; the sector to be read
bOdd                    db  0               ; odd or even
dwKernelSize            dd  0               ; KERNEL.BIN size

; strings
KernelFileName          db  "KERNEL  BIN", 0    ; name of kernel
MessageLength           equ 9
LoadMessage             db  "Loading  "
Message1                db  "Ready.   "
Message2                db  "No KERNEL"


; -------------------------------------------------------------
; DispStr - print out a string(0, 1, 2)
; -------------------------------------------------------------
DispStr:
    mov     ax, MessageLength
    mul     dh
    add     ax, LoadMessage
    mov     bp, ax
    mov     ax, ds
    mov     es, ax  ; es:bp: the position of the string
    mov     cx, MessageLength   ; cx: the length of the string
    mov     ax, 0x1301
    mov     bx, 0x0007
    mov     dl, 0
    add     dh, 3
    int     0x10
    ret

; -------------------------------------------------------------
; ReadSector - read cl sector from ax, to es:bx
; -------------------------------------------------------------
ReadSector:
    push    bp
    mov     bp, sp
    sub     esp, 2  ; 2 byte stack region to save sector number to read

    mov     byte[bp - 2], cl
    push    bx
    mov     bl, [BPB_SecPerTrk]
    div     bl  ; al / bl, al = al / bl, ah = al % bl
    inc     ah
    mov     cl, ah  ; cl: start sector
    mov     dh, al
    shr     al, 1   ; y / BPB_NumHeads
    mov     ch, al  ; 
    and     dh, 1   ; dh & 1, header number
    pop     bx
    mov     dl, [BS_DrvNum] ; drive number
.GoOnReading:
    mov     ah, 2   ; read
    mov     al, byte [bp - 2] ; sectors to be read
    int     13h
    jc      .GoOnReading    ; if carry is set -> failed -> go on reading

    add     esp, 2
    pop     bp

    ret

; -------------------------------------------------------------
; GetFATEntry: in the stack: the cluster of the sector
; -------------------------------------------------------------
GetFATEntry:
    push    es
    push    bx
    push    ax
    mov     ax, BaseOfKernelFile
    sub     ax, 0100h   ; reside some space to save FAT1
    mov     es, ax
    pop     ax  ; ax - the cluster of the entry
    mov     byte [bOdd], 0
    mov     bx, 3
    mul     bx  ; dx:ax = ax * bx
    mov     bx, 2
    div     bx  ; dx:ax / 2 = ax, dx:ax % 2 = dx
                ; 2 FAT Entry -> 3 bytes
    cmp     dx, 0
    jz      LABEL_EVEN
    mov     byte [bOdd], 1  ; set odd
LABEL_EVEN:
    ; now AX: FATEntry offset in FAT, actually in byte
    xor     dx, dx
    mov     bx, [BPB_BytesPerSec]
    div     bx  ; AX <- the sector number for FATEntry
                ; DX <- the offset to the sector
    push    dx
    mov     bx, 0
    add     ax, SectorNoOfFAT1  ; in FAT1
    mov     cl, 2   ; read 2 
    call    ReadSector

    pop     dx
    add     bx, dx
    mov     ax, [es:bx] ; get the entry
    cmp     byte [bOdd], 1
    jnz     LABEL_EVEN_2
    shr     ax, 4
LABEL_EVEN_2:
    and     ax, 0FFFh

LABEL_GET_FAT_ENTRY_OK:
    pop     bx
    pop     es
    ret

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
    jmp     $

; -------------------------------------------------------------
; KillMotor: stop fda
; -------------------------------------------------------------
KillMotor:
    push    dx
    mov     dx, 0x03F2
    mov     al, 0
    out     dx, al
    pop     dx
    ret

; =============================================================
; 32 Bit Code Segment in loader
; =============================================================
[SECTION .s32]

ALIGN   32

[BITS   32]

%include "lib.inc"

LABEL_PM_START:
    ; initialize segment registers
    mov     ax, SelectorVideo
    mov     gs, ax

    mov     ax, SelectorFlatRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax
    mov     esp, TopOfStack

    push    MemChkTitleOffset
    call    PMDispStr
    add     esp, 4

    call    PrintMemInfo
    call    DispReturn

    call    SetupPaging

    ; All set, bring the kernel to memory
    call    InitKernel

    ; GO to the kernel
    push    KernelJmpMsgOffset
    call    PMDispStr
    add     esp, 4
    jmp     SelectorFlatC:KernelEntryPointPhyAddr
    jmp     $

; -------------------------------------------------------------
; InitKernel - bring the kernel to memory
; -------------------------------------------------------------
InitKernel:
    xor     esi, esi
    mov     cx, word [BaseOfKernelFilePhyAddr + 0x2C]   ; ELF header, the number of entry in program header table
    movzx   ecx, cx    ; Move word to double word with zero-extension.
    mov     esi, [BaseOfKernelFilePhyAddr + 0x1C]   ; ELF header, offset of program header table, which describes sections
    add     esi, BaseOfKernelFilePhyAddr    ; KernelFileBase + offset
.Begin:
    mov     eax, [esi + 0]  ; the type?
    cmp     eax, 0
    jz      .NoAction
    push    dword [esi + 0x10]   ; size
    mov     eax, [esi + 0x04]   ; the offset
    add     eax, BaseOfKernelFilePhyAddr
    push    eax
    push    dword [esi + 0x08]  ; the address to load
    call    Memcpy
    add     esp, 12
.NoAction:
    add     esi, 0x20
    dec     ecx
    jnz     .Begin

    ret

; -------------------------------------------------------------
; SetupPaging - set up paging based on current available memory
; -------------------------------------------------------------
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
    push    ecx

    ; Initialize page dir
    mov     ax, SelectorFlatRW
    mov     es, ax
    mov     edi, PageDirBase
    xor     eax, eax
    mov     eax, PageTblBase | PG_P | PG_USU | PG_RWW

.1:
    stosd
    add     eax, 4096
    loop    .1

    ; Initialize Page Table
    pop     eax
    mov     ebx, 1024                        ; 1024 Page Table Entries, 1024 Page Tables
    mul     ebx     ; total: eax * 1024 page tables
    mov     ecx, eax
    mov     edi, PageTblBase
    xor     eax, eax
    mov     eax, PG_P | PG_USU | PG_RWW             ; Base Address -> 0

.2:
    stosd
    add     eax, 4096
    loop    .2

    mov     eax, PageDirBase
    mov     cr3, eax                                ; Move PageDirBase to cr3 without modifying any params
    mov     eax, cr0
    or      eax, 80000000h
    mov     cr0, eax                                ; Enable Paging
    jmp     short .3
.3:
    nop

    push    pagingOKMsgOffset
    call    PMDispStr
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

    push    szRAMSizeOffset
    call    PMDispStr
    add     esp, 4

    push    dword [dwMemSizeOffset]
    call    DispInt
    add     esp, 4

    pop     ecx
    pop     edi
    pop     esi
    ret


; =============================================================
; 32 Bit Data Section
; =============================================================
[SECTION .data1]
ALIGN   32
[BITS 32]
; Some messages
pagingOKMsg             db      "- Done Set up paging.", 0Ah, 0
pagingOKMsgOffset       equ     pagingOKMsg + BaseOfLoaderPhyAddr

KernelJmpMsg            db      "- Kernel Loaded to memory, jumping to it", 0Ah, 0
KernelJmpMsgOffset      equ     KernelJmpMsg + BaseOfLoaderPhyAddr

; Memory Check Buffer
MemChkBuf:
    times   512         db      0
MemChkBufOffset         equ     MemChkBuf + BaseOfLoaderPhyAddr

MemChkTitle:
    db  "- Start Memory Check..........", 0Ah
    db  "BaseAddrL  BaseAddrH LengthLow LengthHigh    Type   ACPI3.x", 0Ah, 0
MemChkTitleOffset       equ     MemChkTitle + BaseOfLoaderPhyAddr

ARDStruct:
    dwBaseAddrLow:      dd      0
    dwBaseAddrHigh:     dd      0
    dwLengthLow:        dd      0
    dwLengthHigh:       dd      0
    dwType:             dd      0
    acpi_resp:          dd      0   ; ACPI responce
ARDStructOffset         equ     ARDStruct + BaseOfLoaderPhyAddr
    dwBaseAddrLowOffset equ     dwBaseAddrLow + BaseOfLoaderPhyAddr
    dwBaseAddrHighOffset equ    dwBaseAddrHigh + BaseOfLoaderPhyAddr
    dwLengthLowOffset   equ     dwLengthLow + BaseOfLoaderPhyAddr
    dwLengthHighOffset  equ     dwLengthHigh + BaseOfLoaderPhyAddr
    dwTypeOffset        equ     dwType + BaseOfLoaderPhyAddr
    dwAcpiResp          equ     acpi_resp + BaseOfLoaderPhyAddr

dwMemSize:
                        dd      0
dwMemSizeOffset         equ     dwMemSize + BaseOfLoaderPhyAddr

szRAMSize:  			db	    "- RAM size:", 0
szRAMSizeOffset         equ     szRAMSize + BaseOfLoaderPhyAddr

; other macros
_szReturn:			    db	        0Ah, 0
_dwDispPos:			    dd	        (80 * 6 + 0) * 2	; 屏幕第 1 行, 第 0 列。
szReturn		        equ	        _szReturn	+ BaseOfLoaderPhyAddr
dwDispPos               equ         _dwDispPos + BaseOfLoaderPhyAddr

MemNum:                 dd          0
MemNumOffset            equ         MemNum + BaseOfLoaderPhyAddr

_PageTableNumber        dd          0
PageTableNumber         equ         _PageTableNumber + BaseOfLoaderPhyAddr

; stack here
StackSpace:             times       1024    db  0
TopOfStack              equ         BaseOfLoaderPhyAddr + $
; End of [SECTION .data]