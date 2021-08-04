section .multiboot_header_back
header_start:
  dd 0xE85250D6                   ; The magic number
  dd 0                            ; Architecture: protected mode i386
  dd header_end - header_start    ; Header length

  ; Checksum
  dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

  ; Other optional multiboot tags

  ; Required end tag, type 0 and size 8
  dw 0                            ; Type
  dw 0                            ; flags
  dd 8                            ; size
header_end:
