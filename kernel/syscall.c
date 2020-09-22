///===- syscall - System Call ---------------------------------------------===//
///
/// Implements system call.
///
///===---------------------------------------------------------------------===//

#include "fb.h"
#include "type.h"

void sys_call_master(u32 x) {
  print("hello sys_call: ");
  fb_print_dec(x);
}