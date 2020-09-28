//===- string.h - String interface ---------------------------------------===//
//
// String operation interfaces.
//
//===---------------------------------------------------------------------===//
#ifndef STRING_H
#define STRING_H

#include "const.h"
#include "type.h"

/// Copy the string from \p src to \p dst
PUBLIC void strcpy(char *dst, char *src);

/// Concatenate two strings, from \p dest to \p src.
PUBLIC void strcat(char *dest, const char *src);

/// Get the length of the string \p str.
PUBLIC unsigned int strlen(char *str);

/// Check if the char \p c is a digit.
PUBLIC bool isdigit(char c);

#endif
