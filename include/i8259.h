/* Interrupt Related Headers */
#ifndef ISR_H
#define ISR_H

#include "const.h"

PUBLIC void init_8259A(void); // init PIC

int clock_int_enter_time;   // records the count of the clock isr

#endif