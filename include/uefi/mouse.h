///===- mouse.h - Mouse related implementations --------------------------===///
/// Implements basic PS/2 mouse functions
///===--------------------------------------------------------------------===///
///
/// Unlike keyboard, the PS/2 mouse needs to be enabled and initialized before
/// we can actually get interrupts from it.
///
///===--------------------------------------------------------------------===///

#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

void ProcessMousePacket();
void PS2MouseInit();
void handlePS2Mouse(uint8_t data);

#define PS2XSign 0b00010000
#define PS2YSign 0b00100000
#define PS2XOverflow 0b01000000
#define PS2YOverflow 0b10000000
#define PS2LBUTTON 0b00000001
#define PS2MBUTTON 0b00000010
#define PS2RBUTTON 0b00000100

#endif // MOUSE_H