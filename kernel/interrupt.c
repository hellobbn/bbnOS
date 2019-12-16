/* For all GDT / IDT settings */
#include "dt.h"
#include "fb.h"
#include "i8259.h"

/** init_idt_desc:
 *  register the handler to the IDT table
 *
 *  @param vector   the interrupt vector
 *  @param desc_type the flag
 *  @param handler  the handler
 *  @param privilege the privilege level
 */
PRIVATE void init_idt_desc(unsigned char vector, u8 desc_type,
                           int_handler handler, unsigned char privilege) {
    GATE *p_gate = &idt[vector];
    u32 base = (u32)handler;
    p_gate->base_low = base & 0xFFFF;
    p_gate->sel = SELECTOR_KERNEL_CS;
    p_gate->base_high = (base >> 16) & 0xFFFF;
    p_gate->flags = desc_type | (privilege << 5);
}

/** exception_handler:
 *  general function to handle interrupts
 *  params are all from stack, see docs for information
 *
 *  TODO: color printing
 */
PUBLIC void exception_handler(int vec_no, unsigned int error_code, int eip,
                              int cs, int cflags) {
    char *err_msg[] = {"#DE Devide Error",
                       "#DB RESERVED",
                       "--  NMI Interrupt",
                       "#BP Breakpoint",
                       "#OF Overflow",
                       "#BR BOUND Range Exceeded",
                       "#UD Invalid Opcode (Undefined Opcode)",
                       "#NM Device Not Available (No Math Coprocessor)",
                       "#DF Double Fault",
                       "    Coprocessor Segment Overrun (reserved)",
                       "#TS Invalid TSS",
                       "#NP Segment not Present",
                       "#SS Stack-Segment Fault",
                       "#GP General Protection",
                       "#PF Page Fault",
                       "--  (Intel Reserved. Do not use.)",
                       "#MF x87 FPU Floating-Point Error (Math Fault)",
                       "#AC Alignment Check",
                       "#MC Machine Check",
                       "#XF SIMD Floating-Point Exception"};

    fb_print("", 1);
    for (int i = 0; i < 80 * 10; ++i) {
        // clear first 5 lines
        print(" ");
    }

    fb_print_color("Exception! --> ", 1, FB_CLOR_RED, FB_CLOR_BLACK);
    fb_print_color(err_msg[vec_no], 0, FB_CLOR_RED, FB_CLOR_BLACK);
    print("\n");
    fb_print_color("------------------------------", 0, FB_CLOR_RED,
                   FB_CLOR_BLACK);
    print("\n") print("EFLAGS: ");
    fb_print_hex(cflags);
    print("\n");
    print("CS: ");
    fb_print_hex(cs);
    print("\n") print("EIP: ");
    fb_print_hex(eip);
    print("\n");

    if (error_code != 0xFFFFFFFF) {
        fb_print("Error Code: ", 0);
        fb_print_hex(error_code);
        print("\n")
    }
    fb_print_color("----------- PANIC ------------", 0, FB_CLOR_RED,
                   FB_CLOR_BLACK);
}

/**
 * extern functions here
 * defined in kernel.asm
 */
void divide_error();
void single_step_error();
void nmi();
void breakpoint_exception();
void overflow();
void bounds_check();
void inval_opcode();
void copr_not_available();
void double_fault();
void copr_seg_overrun();
void inval_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void copr_error();
void exception();

/** init_prot:
 *  Fill in some handlers
 */
PUBLIC void init_prot(void) {
    init_8259A();

    init_idt_desc(INT_VECTOR_DIVIDE, DA_386IGate, divide_error, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_DEBUG, DA_386IGate, single_step_error,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_NMI, DA_386IGate, nmi, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_OVERFLOW, DA_386IGate, overflow, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_BOUNDS, DA_386IGate, bounds_check, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_INVAL_OP, DA_386IGate, inval_opcode,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_COPROC_NOT, DA_386IGate, copr_not_available,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, double_fault,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGate, inval_tss, PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGate, stack_exception,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_PROTECTION, DA_386IGate, general_protection,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault,
                  PRIVILEGE_KRNL);
    init_idt_desc(INT_VECTOR_COPROC_ERR, DA_386IGate, copr_error,
                  PRIVILEGE_KRNL);
}