SelectorKernelCS        equ     8

; import function
extern  k_start_msg ; kmain.c
extern  exception_handler   ; kmain.c
extern  cstart      ; kmain.c
extern  kmain    ; kmain.c
extern  printTestmsg    ; test.c
extern  delay   ; time.c

; global value
extern  gdt_ptr ; kmain.c
extern  idt_ptr ; kmain.c
extern  tss     ; thread.h
extern  p_proc_ready
extern  clock_int_enter_time
extern  clock_handler
extern  irq_table

%include "mem.inc"

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
    ;jmp     0x40:0  ; another exception
    push    0
    popfd

    xor     eax, eax
    mov     ax, SELECTOR_TSS
    ltr     ax

    ;sti
    jmp     kmain
    jmp     $   ; never here

; =============================================================
; ISR HERE
; TODO: move to a new file
; =============================================================

; -------------------------------------------------------------
; Execption handling function
; -------------------------------------------------------------
; Exceptions are hanled by halting the system and displaying
; error messages
global divide_error             ; Divide Error
global single_step_error        ; Debug
global nmi                      ; NMI Interrupt
global breakpoint_exception     ; Breakpoint
global overflow                 ; Overflow
global bounds_check             ; BOUND Range Exceeded
global inval_opcode             ; Invalid Opcode (Undefined Opcode)
global copr_not_available       ; Device Not Available (No Math Coprocessor)
global double_fault             ; Double Fault
global copr_seg_overrun         ; Coprocessor Segment Overrun (reserved)
global inval_tss                ; Invalid TSS
global segment_not_present      ; Segment Not Present
global stack_exception          ; Stack Segment Falut
global general_protection       ; General Protection
global page_fault               ; Page Fault
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

; -------------------------------------------------------------
; Handlers for Hardware Interrupts
; -------------------------------------------------------------
; Those are IRQs for "hardware interrupt", which is generated
; externally by chipset
extern  spurious_irq    ; the outer IRQ

global  hwint0
global  hwint1
global  hwint2
global  hwint3
global  hwint4
global  hwint5
global  hwint6
global  hwint7
global  hwint8
global  hwint9
global  hwint10
global  hwint11
global  hwint12
global  hwint13
global  hwint14
global  hwint15

%macro hwint_master 1
ALIGN 16
hwint%1:
    call    save

    in      al, 0x21
    or      al, (1 << %1)
    out     0x21, al        ; mask the interrupt

    mov     al, 0x20
    out     0x20, al    ; EOI to master

    sti     ; disable interrupt
    push    %1
    call    spurious_irq    ; this function will determine which handler to call
    pop     ecx
    cli

    in      al, 0x21
    and     al, ~(1 << %1)
    out     0x21, al
    ret
%endmacro

ALIGN 16
hwint_master    0   ; clock
hwint_master    1   ; keyboard
hwint_master    2   ; cascade!
hwint_master    3   ; second serial
hwint_master    4   ; first serial
hwint_master    5   ; XT winchester
hwint_master    6   ; floppy
hwint_master    7   ; printer

%macro hwint_slave  1
ALIGN 16
hwint%1:
    push    %1
    call    spurious_irq
    add     esp, 4
    hlt
%endmacro

hwint_slave     8   ; realtime clock
hwint_slave     9   ; irq 2 redirected
hwint_slave     10  ; irq 10
hwint_slave     11  ; irq 11
hwint_slave     12  ; irq 12
hwint_slave     13  ; irq 13
hwint_slave     14  ; irq 14
hwint_slave     15  ; irq 15

global restart
restart:
    mov     esp, [p_proc_ready]
    lldt    [esp + P_LDT_SEL]
    ; move [esp + p_stack_top], which is the end of the stackframe to TSS
    ; (ring 0) so when next interrupt happens, registers are pushed to stack
    ; sequencely
    lea     eax, [esp + P_STACK_TOP]    ; move esp + P_STACK_TOP to eax
    mov     dword [tss + TSS3_S_SP0], eax
restart_reenter:
    dec     dword [clock_int_enter_time]
    pop     gs
    pop     fs
    pop     es
    pop     ds
    popad

    add     esp, 4  ; ignore retaddr
    iretd

; -------------------------------------------------------------
; save register state
; -------------------------------------------------------------
save:
    pushad
    push    ds
    push    es
    push    fs
    push    gs  ; protect some registers
    mov     dx, ss
    mov     ds, dx
    mov     es, dx

    mov     esi, esp    ; save current esp

    inc     dword [clock_int_enter_time]
    cmp     dword [clock_int_enter_time], 0
    ;jne     .re_enter
    jne     .1  ; re-enter

    mov     esp, StackTop

    push    restart ; jmp to restart
    jmp     [esi + RETADR - P_STACKBASE]    ; jmp to restart

.1: ; routine for reenter
    push    restart_reenter ; for re-enter
    jmp     [esi + RETADR - P_STACKBASE]

; no ret: use jmp to jump back, instead of esp, because esp may be changes

; -------------------------------------------------------------
; sys_call handler
; -------------------------------------------------------------
global  sys_call
extern  sys_call_master
sys_call:
    call    save

    sti

    push    eax
    call    sys_call_master
    add     esp, 4
    mov     [esi + EAXREG - P_STACKBASE], eax

    cli

    ret
