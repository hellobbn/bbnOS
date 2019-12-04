; asmsyntax=nasm

; -------------------------------------------------------------
; Some macro
; -------------------------------------------------------------
BaseOfStack         equ     0x7C00  ; grow low, base of stack
BaseOfLoader        equ     0x9000  ; LOADER.BIN loaded here - segment
OffsetOfLoader      equ     0x100   ; LOADER.BIN loaded here - offset
RootDirSectors      equ     14      ; root space
SectorNoOfRootDir   equ     19      ; the first sector of root directory

; -------------------------------------------------------------
; header of FAT12
; -------------------------------------------------------------
jmp short LABEL_START   ; start to boot
nop ; nop

BS_OEMNAME              db          'ForrestY'  ; OEM string, 8 bytes
BPB_BytesPerSec         dw          512 ; bytes each sector
BPB_SecPerClus          db          1   ; sector per cluster
BPB_RsvdSecCnt          dw          1   ; sector for Boot record 
BPB_NumFATs             db          2   ; number of FATs
BPB_RootEntCnt          dw          224 ; max file entry in root
BPB_TotSec16            dw          2880    ; number of logic sectors
BPB_Media               db          0xF0    ; media descriptor
BPB_FATSz16             dw          9   ; sector each fat
BPB_SecPerTrk           dw          18  ; sector each track
BPB_NumHeads            dw          2   ; header number
BPB_HiddSec             dd          0   ; hidden sector
BPB_TotSec32            dd          0   ; 
BS_DrvNum               db          0   ; int13 driver number
BS_Reservedl            db          0   ; reserved
BS_BootSig              db          29h ; extended symbol
BS_VolID                dd          0   ; Volume serial
BS_VolLab               db          'bbnOS_v0.1 '
BS_FileSysType          db          'FAT12   '

; =============================================================
; Start of BOOT.ASM
; =============================================================
LABEL_START:
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, BaseOfStack

    xor     ah, ah  ; ah = 00h
    xor     dl, dl  ; driver = 0
    int     13h ; reset drive

    ; Find LOADER.BIN in A Drive
    mov     word [wSectorNo], SectorNoOfRootDir
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
    cmp     word [wRootDirSizeForLoop], 0   ; chack if root is read over
    jz      LABEL_NO_LOADER
    dec     word [wRootDirSizeForLoop]
    mov     ax, BaseOfLoader
    mov     es, ax  ; es <- Base of loader
    mov     bx, OffsetOfLoader  ; bx is now the offset of loader
    mov     ax, [wSectorNo] ; ax <- some sector in root
    mov     cl, 1
    call    ReadSector  ; data -> es:bx

    mov     si, LoaderFileName  ; ds:si <- "LOADER  BIN"
    mov     di, OffsetOfLoader  ; es:di <- BaseOfLoader:0100
    cld
    mov     dx, 0x10    ; each entry: 0x20 long
LABEL_SEARCH_FOR_LOADER:
    cmp     dx, 0
    jz      LABEL_NEXT_SEARCH   ; not in this sector, next
    dec     dx
    mov     cx, 11
LABEL_CMP_FILENAME:
    cmp     cx, 0
    jz      LABEL_LOADER_FOUND  ; found loader
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
    mov     si, LoaderFileName
    jmp     LABEL_SEARCH_FOR_LOADER
LABEL_NEXT_SEARCH:
    add     word [wSectorNo], 1
    jmp     LABEL_SEARCH_IN_ROOT_DIR_BEGIN
LABEL_NO_LOADER:
    mov     dh, 2   ; "no loader"
    call    DispStr
    jmp     $   ; sad
LABEL_LOADER_FOUND:
    mov     dh, 0
    call    DispStr
    jmp     $   ; stop here

; -------------------------------------------------------------
; Some variables
; -------------------------------------------------------------
wRootDirSizeForLoop dw  RootDirSectors
wSectorNo           dw  0
b0dd                db  0

; string
LoaderFileName      db  "LOADER  BIN", 0
MessageLength       equ 9
BootMessage         db  "Booting  "
Message1            db  "Ready.   "
Message2            db  "NO LOADER"

; -------------------------------------------------------------
; DispStr - print out a string(0, 1, 2)
; -------------------------------------------------------------
DispStr:
    mov     ax, MessageLength
    mul     dh
    add     ax, BootMessage
    mov     bp, ax
    mov     ax, ds
    mov     es, ax  ; es:bp: the position of the string
    mov     cx, MessageLength   ; cx: the length of the string
    mov     ax, 0x1301
    mov     bx, 0x0007
    mov     dl, 0
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

times   510 - ($ - $$)      db  0
dw  0xAA55  ; header