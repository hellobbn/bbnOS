//===- interrupt.h - interrupt interfaces --------------------------------===//
//
// Defines interrupt interfaces
//
//===---------------------------------------------------------------------===//
#ifndef ISR_H
#define ISR_H

#include "const.h"

/// Remap the PIC so that interrupts will not conflict.
/// Then init irq table.
PUBLIC void init_8259A(void);

/// The IRQ handler function pointer.
typedef void (*irq_handler)(int irq);

/// Total 16 IRQs
#define NUM_IRQ 16

/// Register the IRQ handler \p handler to specific \p irq
void register_handler(irq_handler handler, int irq);

/// Disable the \p irq for some reason (don't care)
void disable_irq(int irq);

/// Enable the \p irq
void enable_irq(int irq);

/// IRQ Handlers
///{

/// The clock handler that handles IRQ 0.
void clock_handler(int irq);

/// The keyboard handler to handle IRQ 1.
void keyboard_handler(int irq);

///}

/// IRQ handler for finding the right handler.
void spurious_irq(int irq);

/// Common IRQ handler.
/// This handler is used if no specific handler is specified to the \p irq.
PUBLIC void common_irq(int irq);

/// System call entry, in ASM
void sys_call(void);

/// System call entry, in C
void sys_call_master(void);

/// 0x90 is the interrupt number for system call
#define INT_VECTOR_SYSCALL 0x90

#endif
