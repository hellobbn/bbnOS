///===- syscall - System Call ---------------------------------------------===//
///
/// Implements system call.
///
///===---------------------------------------------------------------------===//

#include "fb.h"
#include "type.h"

void sys_call_master(u32 x) {
  printf("hello sys_call: %X", x);
}