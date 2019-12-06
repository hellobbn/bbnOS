; Header for FAT12

; -------------------------------------------------------------
; The header 
; -------------------------------------------------------------
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

; -------------------------------------------------------------
; Some defs = note: should be changed if the header is changed
; -------------------------------------------------------------
FATsz                   equ         9
RootDirSectors          equ         14
SectorNoOfRootDirectory equ         19
SectorNoOfFAT1          equ         1
DeltaSectorNo           equ         17