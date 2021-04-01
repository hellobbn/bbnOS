///===- string.h - String Operations ------------------------------------===///
/// Generic operations for strings.
///===-------------------------------------------------------------------===///
///
/// This header defines generic string operations which are used in kernel.
/// Most of those functions may NOT be safe, meaning that there might be memory
/// leak if not used properly, which will eventually cause kernel PANIC. So use
/// them in caution!
///
///===-------------------------------------------------------------------===///

#ifndef STRING_H
#define STRING_H

#include "types.h"

/// Copy the string from start to end. NOT memory safe
///
/// \param dst The destination
/// \param src The source string
void strcpy(const char *src, char *dst);

/// Concatenate two strings. NOT memory safe
///
/// \param src The source string
/// \param dst The destination, the source string will be appendded at the end
void strcat(const char *src, char *dest);

/// Get the length of string, NOT memory safe
///
/// \param str The input string
/// \return The length of the string
size_t strlen(const char *str);

/// Check if the character is a digit
///
/// \param c The character
/// \return 1 of it is a digit, 0 if not
int isdigit(char c);

#endif // STRING_H

