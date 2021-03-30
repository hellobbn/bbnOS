//===- global.h: global variables ----------------------------------------===//
//
// Defines global variables. We move the variables that share across the c
// files to this fils so that those variables are only defined once.
//
//===---------------------------------------------------------------------===//

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef GLOBAL_VAR_HERE
#ifdef EXTERN
#undef EXTERN
#endif
#define EXTERN
#endif

#include "const.h"
#include "interrupt.h"
#include "thread.h"
#include "type.h"

/// Task table
EXTERN TASK task_table[MAX_THREAD];

/// Stack
EXTERN char task_stack[STACK_SIZE_TOTAL];

/// TSS
EXTERN TSS tss;

/// Current process
EXTERN PROCESS *p_proc_ready;

/// The process table
EXTERN PROCESS proc_table[MAX_THREAD];

/// IDT Functions and declarations
///{
EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE];
///}

/// The count of the clock isr
EXTERN u32 clock_int_enter_time;

/// IRQ handler
EXTERN irq_handler irq_table[NUM_IRQ];

#endif
