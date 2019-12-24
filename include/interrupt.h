/* Interrupt Related Headers */
#ifndef ISR_H
#define ISR_H

#include "const.h"

PUBLIC void init_8259A(void); // init PIC

typedef void (*irq_handler) (int irq);  // function pointer

#define NUM_IRQ 16  // total 16 IRQs

void register_handler(irq_handler handler, int irq);

void disable_irq(int irq);  // disable IRQ for some reason(don't care)

void enable_irq(int irq);   // enable IRQ

void clock_handler(int irq);

void spurious_irq(int irq); // irq handler for finding right handler

PUBLIC void common_irq(int irq);    // common irq handler

// system call

void sys_call(void);    // system call entry, in asm

void sys_call_master(void); // system call entry, in c

#define INT_VECTOR_SYSCALL  0x90

#endif