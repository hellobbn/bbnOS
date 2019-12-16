SelectorKernelCS        equ     8

; import function
extern  k_start_msg ; start.c
extern  exception_handler   ; protect.c
extern  cstart      ; start.c

; global value
extern  gdt_ptr ; from start.c
extern  idt_ptr ; from start.c

; =============================================================
; BSS SECTION
; =============================================================
[SECTION .bss]
StackSpace              resb        2 * 1024
StackTop:               ; top of stack

; =============================================================
; TEXT SECTION
; =============================================================
[SECTION .text]

global _start

; -------------------------------------------------------------
; KERNEL start here 
; -------------------------------------------------------------
_start:
    call    k_start_msg ; print kernel message
    mov     esp, StackTop

    sgdt    [gdt_ptr]   ; save the loader's GDT ptr to the given point
    call    cstart      ; copy the content of GDT
    lgdt    [gdt_ptr]   ; reload GDT ptr

    lidt    [idt_ptr]

    jmp     SelectorKernelCS:csinit

csinit:
    ;ud2     ; force a exception
    jmp     0x40:0  ; another exception
    push    0
    popfd

    jmp     $

; -------------------------------------------------------------
; Interrupt handling function
; -------------------------------------------------------------
global divide_error
global single_step_error
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error
global exception

divide_error:
    push    0xFFFFFFFF
    push    0
    jmp     exception
single_step_error:
    push    0xFFFFFFFF
    push    1
    jmp     exception
nmi:
    push    0xFFFFFFFF
    push    2
    jmp     exception
breakpoint_exception:
    push    0xFFFFFFFF
    push    3
    jmp     exception
overflow:
    push    0xFFFFFFFF
    push    4
    jmp     exception
bounds_check:
    push    0xFFFFFFFF
    push    5
    jmp     exception
inval_opcode:
    push    0xFFFFFFFF
    push    6
    jmp     exception
copr_not_available:
    push    0xFFFFFFFF
    push    7
    jmp     exception
double_fault:
    push    8
    jmp     exception
copr_seg_overrun:
    push    0xFFFFFFFF
    push    9
    jmp     exception
inval_tss:
    push    10
    jmp     exception
segment_not_present:
    push    11
    jmp     exception
stack_exception:
    push    12
    jmp     exception
general_protection:
    push    13
    jmp     exception
page_fault:
    push    14
    jmp     exception
copr_error:
    push    0xFFFFFFFF
    push    16
    jmp     exception
exception:
    call    exception_handler
    add     esp, 4 * 2  ; top of stack -> eip
    hlt