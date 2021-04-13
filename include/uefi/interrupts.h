
///===- interrupts.h -  Interrupts and Exceptions ------------------------===///
/// General interrupts declaration
///===--------------------------------------------------------------------===///
///
/// Based on intel 64 and IA-32 manual, there are 256 interruptes, where there
/// are 22 exceptions indexed from 0 to 21, and 10 reserved by intel (22-31),
/// and the remainders are user defined interrupts (External interrupts or
/// INT n) instruction.
///
///===--------------------------------------------------------------------===///

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/// Pre-defined Exceptions, the vector is 0 (#DE) to 21 (#CP)
///{
#define NUM_EXCEPTIONS 22
enum PROCESSOR_EXCEPTIONS {
  EX_DEVIDE_ERROR = 0,
  EX_DEBUG_EXCEPTION = 1,
  EX_NMI = 2,
  EX_BREAKPOINT = 3,
  EX_OVERFLOW = 4,
  EX_BOUND_RANGE_EXCEEDED = 5,
  EX_INVALID_OPCODE = 6,
  EX_DEVICE_NOT_AVAI = 7,
  EX_DOUBLE_FAULT = 8,
  EX_COPROCESSOR_SEG_OVERRUN = 9,
  EX_INVALID_TSS = 10,
  EX_SEG_NOT_PRESENT = 11,
  EX_STACK_SEG_FAULT = 12,
  EX_GENERAL_PROTECTION = 13,
  EX_PAGE_FAULT = 14,
  EX_RESERVED = 15,
  EX_X87_FPU_FP_ERROR = 16,
  EX_ALIGNMENT_CHECK = 17,
  EX_MACHINE_CHECK = 18,
  EX_SIMD_FP_EXCEPTION = 19,
  EX_VIRT_EXCEPTION = 20,
  EX_CONTROL_PROTECTION_EXCEPTION = 21,
};

extern char *PROCESSOR_EXCEPTION_STRING[NUM_EXCEPTIONS];
///}

struct interrupt_frame;

/// Exception Handlers
///{
__attribute__((interrupt)) void exHandlerDF(struct interrupt_frame *frame);
__attribute__((interrupt)) void exHandlerGP(struct interrupt_frame *frame);
__attribute__((interrupt)) void exHandlerPF(struct interrupt_frame *frame);
///}

/// Interrupt Handlers
///{
__attribute__((interrupt)) void intHandlerKB(struct interrupt_frame *frame);
__attribute((interrupt)) void intHandlePS2Mouse(struct interrupt_frame *frame);
///}

#endif // INTERRUPTS_H