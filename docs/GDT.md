# GDT - Global Descriptor Table

## A. Overview

In real mode, although we have 32-bit registers which can access address up to 4 GB,
we still use `segment:offset` method to access memory, the `XXXXh` in segment means
memory starting from `XXXX0h`. In protected mode, we still use registers like 
`cs`, `ds`, but they are becoming indexes, pointing to a `descriptor` in a data structure,
which carefully defines the `base`, `limit`, `attr` of the segment. The data structure is 
named `GDT` (maybe `LDT`).

## B. Code / Data segment Descriptor

```text
HIGH ---------------------------------------------------------------- LOW
| Byte 7 | Byte 6 | Byte 5 | Byte 4 | Byte 3 | Byte 2 | Byte 1 | Byte 0 |
| Base 2 |     Attr (1)    |          Base 1          |     Limit 1     |
```

__Note__: `Base 2` contains 31...24 bit of `Base` while `Base 1` contains 23....0 of `Base` (Total 32 bits)

__Note 2__: `Limit 1` contains 15...0 bit of `Limit` (Total 20 bits)

In `boot.inc` we can see the `Descriptor` macro defined like this:

```asm
%macro Descriptor 3
	dw	%2 & 0FFFFh				                ; Limit 1
	dw	%1 & 0FFFFh				                ; Base 1 (15 - 0)
	db	(%1 >> 16) & 0FFh			            ; Base 1 (23 - 16)
	dw	((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)	    ; Attr 
	db	(%1 >> 24) & 0FFh			            ; Base 2
%endmacro                                       ; 8 Bytes Total
```

__(1)__ Attr Table:

```text
| 7 |  6  | 5 |  4  | 3 2 1 0 | 7 | 6 5 | 4 | 3 2 1 0|
| G | D/B | 0 | AVL | Limit 2 | P | DPL | S |  TYPE  |
|<---------- Byte 6 --------->|<------ Byte 5 ------>|
```

In `boot.asm` we can see code like this:

```asm
mov         ax, SelectorVideo
mov         gs, ax
```

which moves `SelectorVideo`, defined like this:

```asm 
SelectorVideo       equ     LABEL_DESC_VIDEO  - LABEL_GDT       ; 16
``` 

to  `gs`. But it is actually NOT the offset from `LABEL_DESC_VIDEO` to `GDT Base`. It is a `Selector`, whose structure is like the following:

```text
|     15 ... 3     | 2  | 1 0 |
| Descriptor Index | TI | RPL |
```

Only when both `TI` and `RPL` becomes `0` can the `Selector` become offset.

In `boot.asm`, we save `ax`, which contains information, to `[gs:edi]`, which is in the VRAM.

