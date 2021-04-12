///===- keycode.h - Key Codes --------------------------------------------===///
/// Predefined keycodes for keyboards
///===--------------------------------------------------------------------===///
///
/// The PS/2 Kayboard talks to a PS/2 controller using serial communication. The
/// PS/2 keyboards accepts commands and sends responses to those commands, and
/// also sends scancodes indicating when a key was pressed or released.
///
/// There are many sets of keyboards, and here, we only define the "US QWERTY"
/// keyboard
///
///===--------------------------------------------------------------------===///

#ifndef KEYCODE_H
#define KEYCODE_H

#include "types.h"

/// Handle the scan code and print the corresponding key to screen
///
/// \param scancode The keyboard scan code
void keyboardHandle(uint8_t scancode);

#endif