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

## C. Jump from Real Mode to Protected Mode

In `boot.asm` Section `.s16`, we can see code like this:

```asm
; Initialize 32-bit Segment Descriptor
    xor     eax, eax                            ; 0 -> EAX
    mov     ax, cs                              ; CS -> AX
    shl     eax, 4                              ; EAX << 4
    add     eax, LABEL_SEG_CODE32
    mov     word [LABEL_DESC_CODE32 + 2], ax    ; AX -> [LAB...] 
    shr     eax, 16
    mov     byte [LABEL_DESC_CODE32 + 4], al
    mov     byte [LABEL_DESC_CODE32 + 7], ah
```

First, we use `Segment:Offset` to assign the address of `LABEL_SEG_CODE32` to `EAX` (2). Then we assign thr value
to the corresponding position in `LABEL_DESC_CODE32`, which 
defines `Limit` and `Attr`.

Now we have successfully Initialized Descriptors! Now we need to 
fill `GDTPtr` with `GDT`'s physical address (`Segment:Offset`). 

Then the instruction `lgdt [GDTPtr]` loads the 6-byte data in `GDTPtr` to `gdtr`. The structure of `gdtr` is decribed below:

```text
|            32-bit base             |      16-bit limit       |
H<------------------------------------------------------------>L
```

Then we disable interrupt because protect mode handles interrupt differently.

Then we open address line A20. We will skip this part.

Then we set the 0th position of `cr0` to 1, which is ctritical when we are going to enter Protect Mode. The `PE` position controls if we are going to run in Protect Mode. When PE == 1, CPU runs in Protect Mode.

Actually, when we finished the `mov cr0, eax`, the system is already running in Protect mode. But the `CS` still contains the 
value in Real Mode. We need to load the code segment selector to CS. So we do the `jmp`

The instruction `jmp SelectorCode32:0` will jump to `LABEL_SEG_CODE32`.

Finally We've GOT to Protect Mode!

__(2)__: In INT 0x10 we only need offset, not the address

## D. Attr in GDT

- __P__: Present Bit. P == 1: This segment exists in Memory.
- __DPL__: Descriptor Privilege Level. 0 / 1 / 2 / 3. Larger means more previleged
- __S__: Data / Code: S = 1 ; System / Gate: S = 0
- __TYPE__: (3)
- __G__: Granularity: G = 0: byte; G = 1: 4KB
- __D/B__: (4)
- __AVL__: Reserved, can be used by system software

In CodeSegment, we see the `Attr` is like `DA_C + DA_32`, where `DA_C` means `98h`, or `10011000b` and `DA_32` means `4000h`, setting the __D__ Position to 1. DPL = 0.

__(3)__: Type Table please see `osdev`, (EX/DC/RW/AC)

| Type |             Data / Segment Descriptor             | System / Gate Descriptor |
|------|:-------------------------------------------------:|:------------------------:|
|   0  |                     Read Only                     |       \<Not Defined>      |
|   1  |                Read Only, Accessed                |       Usable 286TSS      |
|   2  |                    Read / Write                   |            LDT           |
|   3  |               Read / Write, Accessed              |        Busy 286TSS       |
|   4  |               Read Only, Expend-Down              |       286 Call Gate      |
|   5  |          Read Only, Expend-Down, Accessed         |         Work Gate        |
|   6  |             Read / Write, Expend-Down             |    286 Interrupt Gate    |
|   7  |        Read / Write, Expend-Down, Accessed        |       286 Trap Gate      |
|   8  |                    Execute Only                   |       \<Not Defined>      |
|   9  |               Execute Only, Accessed              |       Usable 386TSS      |
|   A  |                   Execute / Read                  |       \<Not Defined>      |
|   B  |              Execute / Read, Accessed             |        Busy 386TSS       |
|   C  |       Execute Only, Conforming Code Segment       |       386 Call Gate      |
|   D  |  Execute Only, Conforming Code Segment, Accessed  |       \<Not Defined>      |
|   E  |      Execute / Read, Conforming Code Segment      |    386 Interrupt Gate    |
|   F  | Execute / Read, Conforming Code Segment, Accessed |       386 Trap Gate      |



__(4)__: D/B Bit, please see `osdev`,(Sz?)