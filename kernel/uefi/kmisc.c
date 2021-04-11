///===- kmisc.c - misc functions -----------------------------------------===///
/// Declares misc functions which can't be easily catagorized
///===--------------------------------------------------------------------===///

#include "kmisc.h"
#include "fb.h"

void kPanic(const char *panic_message) {
  fbClearScreen(0x00ff0000);

  fbResetCursor(0, 0);
  fbSetColor(0);

  printf("Kernel Panic: %s", panic_message);

  while (1) {
  }
}