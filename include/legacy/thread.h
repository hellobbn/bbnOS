//===- thread.h - Thread support -----------------------------------------===//
//
// Thread support interfaces.
//
//===---------------------------------------------------------------------===//
#ifndef THREAD_H
#define THREAD_H

#include "dt.h"
#include "type.h"

/// Max 3 threads for now.
#define MAX_THREAD 3

/// The stack frame.
/// The sequence of the register strictly follows the `pusha` sequence and the
/// cpu.
///
/// Like the following:
/// `pusha`:
///   - temporary = esp
///   - push eax (the first to push)
///   - push ecx, edx, ebx, temporary, ebx, esi, edi
///
/// `cpu`
///   - push ss, esp, cs, eip
///
struct s_stackframe {
  u32 gs;
  u32 fs;
  u32 es;
  u32 ds;
  u32 edi;
  u32 esi;
  u32 ebp;

  // this is ignored py `popd`
  u32 kernel_esp;

  u32 ebx;
  u32 edx;
  u32 ecx;
  u32 eax;

  // Return address for kernel.asm::save
  u32 retaddr;

  u32 eip;
  u32 cs;
  u32 eflags;
  u32 esp;
  u32 ss;
};

/// The stack frame.
typedef struct s_stackframe STACK_FRAME;

/// The thread PCB.
struct s_proc {
  STACK_FRAME registers;

  /// The GDT selector telling the LDT base and limit.
  u16 ldt_sel;

  /// Local descriptor for the code and data.
  DESCRIPTOR ldts[LDT_SIZE];

  /// Process ID.
  u32 pid;

  /// If it has a name...
  char p_name[16];
};

/// PCB Type definition.
typedef struct s_proc PROCESS;

/// Stack test code
///{
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TOTAL                                                       \
  (STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC)
///}

/// from kernel.asm
void restart(void);

/// Task function pointer. Pointing to the function of the task.
typedef void (*task_f)();
/// The task structure.
struct s_task {
  task_f initial_eip;
  unsigned int stack_size;
  char name[16];
};

/// Task type definition
typedef struct s_task TASK;

/// Demo task definitions
///{
// proto-type
PUBLIC void testA();
PUBLIC void testB();
PUBLIC void testC();
///}

#endif
