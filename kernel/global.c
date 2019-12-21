#include "const.h"
#include "thread.h"
#include "dt.h"
#include "i8259.h"

// task-table
PUBLIC TASK task_table[MAX_THREAD] = {{testA, STACK_SIZE_TESTA, "TestA"},
                                      {testB, STACK_SIZE_TESTB, "testB"}};

// the stack
PUBLIC char task_stack[STACK_SIZE_TOTAL];

// the TSS
PUBLIC TSS tss;

// the thread table
PUBLIC PROCESS *p_proc_ready;

// The process table
PUBLIC PROCESS proc_table[MAX_THREAD];

// IDT Functions and declarations 
PUBLIC u8 gdt_ptr[6];
PUBLIC DESCRIPTOR gdt[GDT_SIZE];
PUBLIC u8 idt_ptr[6];
PUBLIC GATE idt[IDT_SIZE]; 
