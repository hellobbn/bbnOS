///===- kmisc.h - misc functions -----------------------------------------===///
/// Declares misc functions which can't be easily catagorized
///===--------------------------------------------------------------------===///
///
/// Function List:
///  - kpanic(): Panic the kernel
///
///===--------------------------------------------------------------------===///

#ifndef KMISC_H
#define KMISC_H

void kPanic(const char *panic_message);

#endif // KMISC_H