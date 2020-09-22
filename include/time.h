//===- time.h - clock functions ------------------------------------------===//
//
// Interfaces for time functions.
//
//===---------------------------------------------------------------------===//
#ifndef TIME_H
#define TIME_H

/// Delay some time, which is geven by \p time.
/// Note that it is not accurate: we don't use the clock IRQ now, but we can
/// use them later.
///
/// This function is, now, CPU-consuming, very expensive.
void delay(int time);

#endif
