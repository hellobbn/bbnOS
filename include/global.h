#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef GLOBAL_VAR_HERE
#undef EXTERN
#define EXTERN
#endif

#include "const.h"
#include "thread.h"
#include "type.h"
#include "interrupt.h"

// task-table
EXTERN TASK task_table[MAX_THREAD];

// the stack
EXTERN char task_stack[STACK_SIZE_TOTAL];

// the TSS
EXTERN TSS tss;

// the thread table
EXTERN PROCESS *p_proc_ready;

// The process table
EXTERN PROCESS proc_table[MAX_THREAD];

// IDT Functions and declarations 
EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE]; 

// records the count of the clock isr
EXTERN int clock_int_enter_time;

// IRQ handler
EXTERN  irq_handler irq_table[NUM_IRQ];

#endif