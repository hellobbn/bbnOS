; The loader
org 0x0100

BaseOfStack         equ     0x0100
BaseOfKernelFile    equ     0x8000  ; kernel.bin should be loaded here
OffsetOfKernelFile  equ     0x0     ; no offset

jmp     LABEL_START

%include "fat12hdr.inc"

LABEL_START:
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, BaseOfStack

    mov     dh, 0
    call    DispStr

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

    mov     dh, 1
    call    DispStr

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
; KillMotor: stop fda
; -------------------------------------------------------------
KillMotor:
    push    dx
    mov     dx, 0x03F2
    mov     al, 0
    out     dx, al
    pop     dx
    ret