///===- string.c - String Op Implementation -----------------------------===///
/// Definitions of generic string operations
///===-------------------------------------------------------------------===///
//
// TODO: The implementation of this string operation is generic, and legacy-
// uefi independent. We need to combine them later
//
//===--------------------------------------------------------------------===///
#include "string.h"

void strcpy(const char *src, char *dst) {
  int i = 0;
  while (src[i]) {
    dst[i] = src[i];
    i++;
  }
}

void strcat(const char *src, char *dest) {
  int destPtr = 0;
  int srcPtr = 0;

  while (dest[destPtr] != 0) {
    destPtr++;
  }

  while (src[srcPtr] != 0) {
    dest[destPtr++] = src[srcPtr++];
  }
  dest[destPtr] = 0;
}

size_t strlen(const char *str) {
  int size = 0;

  while (str[size] != 0) {
    size++;
  }

  return size;
}

int isdigit(char c) {
  if (c >= '0' && c <= '9') {
    return 1;
  }

  return 0;
}
