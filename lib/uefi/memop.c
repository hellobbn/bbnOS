///===- memop.c - Memory Operations -------------------------------------===///
/// This file defines basic memory operations
///===-------------------------------------------------------------------===///
//
// NOTE: The code are copied from the following locations:
//       https://forum.osdev.org/viewtopic.php?f=1&t=32866
//
//===--------------------------------------------------------------------===///

#include "types.h"
#include "memop.h"

void *memset(void *d, int s, size_t c) {
  void *temp = d;

  // This code explanation:
  // Output:
  //   =&D: edi register, & means it cannot be modified before the end of code
  //   =&c: same for ecx register
  // Input:
  //   "0"(d): edi
  //   "a" (s): eax
  //   "1" (c): ecx
  // Repeat ecx times, store eax to edi
  __asm__ volatile("rep stosb"
                   : "=&D"(d), "=&c"(c)
                   : "0"(d), "a"(s), "1"(c)
                   : "memory");
  return temp;
}
