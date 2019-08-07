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
- __DPL__: Descriptor Privilege Level. 0 / 1 / 2 / 3. Larger means more privileged
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

- In Executable Code Descriptor: D bit; D = 1 --> 32-bit address, 32-bit or 8-bit operand ; D = 0 --> 16-bit address, 16-bit or 8 bit operand
- In Expend Down Descriptor: B Bit; B = 1 --> Segment Limit: 4GB; B = 0 --> Segment Limit 64KB
- In STACK Descriptor: B Bit: B = 1 --> implicit stack accessing instruction(`pop`, `push`, `call`) use 32-bit `esp`. B = 0 --> 16-bit `esp`

## E. Privilege Level

In IA32, there are 4 privilege levels: 0, 1, 2, 3. Smaller number indicates higher privilege.
Critical code or data should be put in higher privilege level, and processor is using this kind of strategy to preventing access from lower privilege level to 
higher privilege level. If the processor dectes illegal request, it arises #GP.

### E.1 CPL, DPL, RPL

#### E.1.1 CPL (Current Privilege Level)

`CPL` is the privilege level where current program is in. It is stored in the 1 - 0 bit in `cs` and `ss`. Generally, CPL equals the privilege level where 
current code is in. When switching to another code segment, the processor will change `CPL`.

`Conforming Code Segment` can be accessed by code having the same or lower privilege level, and the CPL will not change.

#### E.1.2 DPL (Descriptor Privilege Level)

`DPL` indicates the privilege level of a `Gate` or `Segment`. It is stored in the `DPL` position in the `Segment` or `Gate` Descriptor. 
When current code wants to access a `Segment` or `Gate`, its `DPL` will be compared to `CPL` and the `Segment` or `Gate`'s `RPL`.

- __Data Segment__: `DPL` defines the lowest privilege level that can access this segment. _example: `DPL` = 1 --> CPL = 0 / 1 can access_
- __Unconforming Code Segment__: `DPL` defines the privilege level that can access this segment. _Must be the same_
- __Call Gate__: `DPL` defines the lowest privilege level that can access this `Call Gate`. _Same as `Data Segmant`_
- __Conforming Code Segment__: (or `Unconforming Code Segment` accessed by `Call Gate`) : `DPL` defines the highest privilege level that
can access this segment. _example: `DPL` = 2 --> CPL = 0 / 1 can't access_
- __TSS__: `DPL` defines the lowest privilege level that can access `TSS`. _Same as `Data Segment`_

#### E.1.3 RPL (Requested Privilege Level)

`RPL` is stored in the 0 - 1 bit in the Selector. The processor decides whether a request is legal by checking `RPL` and `CPL`, meaning even the
segment that is requesting has enough `CPL`, it still needs enough `RPL`. So we need `max(RPL, CPL)`

The Operating System usually use `RPL` to prevent lower privileged application to access higher privileged segment.

### E.2 Privilege Teansfer

#### E.2.1 Using `jmp` and `call`

`jmp` and `call` instruction can directly transfer control. For example, if the target is _Unconforming Code Segment_, then the CPL needs to be equal to the target's `DPL`. If the target is _Conforming Code Segment_, then the target's `CPL` has to be laerger than or equal to thr target's `DPL`. So `jmp` and `call` is very limited. We are needing more methods.

#### E.2.2 Gate

`Gate` is a descriptor, whose structure is like the following:

```
HIGH ---------------------------------------------------------------- LOW
| Byte 7 | Byte 6 | Byte 5 | Byte 4 | Byte 3 | Byte 2 | Byte 1 | Byte 0 |
|     Offset      |       Attr      |     Selector    |      Offset     |

Attr:

| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| P |   DPL | S |     TYPE      | 0 | 0 | 0 |    Param Count    |
```

There are four gates:
 - Call Gates
 - Interrupt Gates
 - Trap Gates
 - Task Gates

As you can see in code, there is a `Call Gate` calling the code in `DESC_CODE_RING3`.

Gate and `call` allow transfer from lower privilege level to higher privilege level.

Note that programs in different privilege level does NOT share the same stack.

#### E.2.3 RET

##### E.2.3.1 About Stack

If a `call` or `jmp` does not happen in the same `SEGMENT`, it is a `Far jmp/call`, instead we cakll it a `Near jmp/call`.

For `jmp`, this does not affect much, it is just a matter of position: One within the segment, one outside the segment.

But for `call`, things gets a bit complicated, because `call` will affect the stack. `Near call` pushes `eip` and pops `eip` when `ret`. `Far call` will not only push `eip` but also `cs`.

##### E.2.3.2 More Theory

As we have mentioned above, `call` through `gate` will deal changes to the `stack`. As we have 4 privilege levels, we need 4 stacks. We therefore need `TSS`(Task-State Stack), a data structure containing many bytes, in which there are 3 `ss` and `esp` pointing to `stack` of other privilege levels.

Here is the prosedure:

1. Based on the target code's `DPL` (New `CPL`), choose the correct `esp` and `ss`

2. Read new `ss` and `esp` from `TSS` (Will raise #TS if wrong ss, esp or TSS limit is found.)

3. Validate `ss` descriptor. Will raise #TS if wrong.

4. Temperarily save current `ss` and `esp`.

5. Load new `ss` and `esp`.

6. Push the `ss` and `esp` just saved.

7. Copy parameters from caller's stack to current stack. Need `Param Count` to decide how many parameters to be copied.

8. Push current `cs` and `eip`

9. Load `cs` and `eip` from gate, start program being called.

---------------------------------------

The Stack will look like this:
```
HIGH
|    ss   |
|    esp  |
| param 1 |
| param 2 |
| param 3 |
|    cs   |
|   eip   |
LOW
```

And for `ret`, here is the precedure:

1. Check `RPL` in the `cs` in the stack to decide if changing privilege level is required

2. Load `cs` and `eip` from stack

3. Add `esp` to skip parameters. `esp` now points to caller's `ss` and `esp`.

4. load `ss` and `esp`. Switch to the stack of the caller. Trash current `ss` and `esp`.

5. In caller's stack, skip the parameters.

6. Check `ds`, `es`, `fs`, `gs`, if any of them's `DPL` is lower than `CPL`(Does NOT apply to `Unconforming Code Segment`), then a empty descriptor will be loaded to the register.