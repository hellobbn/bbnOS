; Some const, using oranges
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               sconst.inc
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

P_STACKBASE	equ	0
GSREG		equ	P_STACKBASE     ; offset 0, GS
FSREG		equ	GSREG		+ 4 ; fs
ESREG		equ	FSREG		+ 4 ; es
DSREG		equ	ESREG		+ 4 ; ds
EDIREG		equ	DSREG		+ 4 ; edi
ESIREG		equ	EDIREG		+ 4 ; esi
EBPREG		equ	ESIREG		+ 4 ; ebp
KERNELESPREG	equ	EBPREG		+ 4 ; kernel_esp, trashed
EBXREG		equ	KERNELESPREG	+ 4 ; ebx
EDXREG		equ	EBXREG		+ 4 ; edx
ECXREG		equ	EDXREG		+ 4 ; ecx
EAXREG		equ	ECXREG		+ 4 ; eax
RETADR		equ	EAXREG		+ 4 ; retaddr
EIPREG		equ	RETADR		+ 4 ; eip
CSREG		equ	EIPREG		+ 4 ; cs
EFLAGSREG	equ	CSREG		+ 4 ; eflags
ESPREG		equ	EFLAGSREG	+ 4 ; esp
SSREG		equ	ESPREG		+ 4 ; ss
P_STACK_TOP	equ	SSREG		+ 4 ; ?
P_LDT_SEL	equ	P_STACK_TOP     ; ldt_sel
P_LDT		equ	P_LDT_SEL	+ 4 ; ldts

TSS3_S_SP0	equ	4

; 以下选择子值必须与 protect.h 中保持一致!!!
SELECTOR_FLAT_C		equ		0x08	; LOADER 里面已经确定了的.
SELECTOR_TSS		equ		0x20	; TSS
SELECTOR_KERNEL_CS	equ		SELECTOR_FLAT_C

