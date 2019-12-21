#ifndef GLOBAL_H
#define GLOBAL_H

#include "const.h"
#include "thread.h"
#include "type.h"

extern TASK task_table[];
extern u8 gdt_ptr[];
extern DESCRIPTOR gdt[];
extern u8 idt_ptr[];
extern GATE idt[];
extern TSS tss;
extern PROCESS proc_table[];
extern PROCESS* p_proc_ready;
extern char task_stack[];

#endif