#include "interrupts.h"
#include "fb.h"

char *PROCESSOR_EXCEPTION_STRING[NUM_EXCEPTIONS] = {
    "#DE Devide Error",
    "#DB Debug Exception",
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
    "#XM SIMD Floating-Point Exception",
    "#VE Virtualization Exception",
    "#CP Control Protection Exception"};

__attribute__((interrupt)) void exHandlerPF(struct interrupt_frame *frame) {

  printf("PageFault detected.\n");

  while (true) {
  }
}