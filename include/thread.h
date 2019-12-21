/** thread.h
 *  header file for Thread and program
 */
#ifndef THREAD_H
#define THREAD_H

#include "dt.h"
#include "type.h"

#define MAX_THREAD 2 // max 1 thread for now

/** s_stackframe: the stack frame;
 *  The sequence of the register strictly follows the pusha sequence
 *  and the cpu
 *
 *  Like the following:
 *
 *  `pusha`
 *    - tmporary = esp
 *    - push eax    ( the first to push )
 *    - push ecx edx, ebx, tmporary, ebp, esi, edi
 *
 *  `cpu`
 *    - push ss, esp, cs, eip
 */
struct s_stackframe {
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp; // this is ignored by popd
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 retaddr; // return address for kernel.asm::save
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
};

typedef struct s_stackframe STACK_FRAME; // the stack frame

/** s_proc: the thread PCB
 */
struct s_proc {
    STACK_FRAME registers;
    u16 ldt_sel;               // The GDT selector telling IDT base and limit
    DESCRIPTOR ldts[LDT_SIZE]; // local descriptor for code and data
    u32 pid;                   // process ID
    char p_name[16];           // if it has a name...
};

typedef struct s_proc PROCESS; // the PCB

#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TOTAL (STACK_SIZE_TESTA + STACK_SIZE_TESTB)

// from kernel.asm
void restart(void);

// s_task: the structure for a task
typedef void (*task_f)(); // function pointer
struct s_task {
    task_f initial_eip;
    int stack_size;
    char name[16];
};

typedef struct s_task TASK;

// define the 2 tasks

// proto-type
PUBLIC void testA();
PUBLIC void testB();

#endif