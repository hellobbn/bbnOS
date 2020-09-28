//===- string.c - string operations ---------------------------------------===//
//
// String operation implementation.
//
//===----------------------------------------------------------------------===//
#include "type.h"

void strcpy(char *dst, char *src) {
  int i = 0;
  while (src[i]) {
    dst[i] = src[i];
    i++;
  }
}

void strcat(char *dest, const char *src) {
  int destPtr = 0;
  int srcPtr = 0;
  
  while (dest[destPtr] != 0) {
    destPtr ++;
  }

  while (src[srcPtr] != 0) {
    dest[destPtr ++] = src[srcPtr ++];
  }
}

unsigned int strlen(char *str) {
  int size = 0;

  while (str[size] != 0) {
    size ++;
  }

  return size;
}

bool isdigit(char c) {
  if (c >= 48 && c <= 57)
    return true;

  return false;
}