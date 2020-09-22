//===- string.c - string operations ---------------------------------------===//
//
// String operation implementation.
//
//===----------------------------------------------------------------------===//
void strcpy(char *dst, char *src) {
  int i = 0;
  while (src[i]) {
    dst[i] = src[i];
    i++;
  }
}