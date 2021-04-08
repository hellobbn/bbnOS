///===- bitmap.c - BitMap -----------------------------------------------===///
/// The bitmap for kernel
///===-------------------------------------------------------------------===///

#include "bitmap.h"
#include "types.h"

bool bitmapGetVal(BitMap *bm, uint64_t index) {
  uint64_t byte_index = index / 8;
  if (byte_index > bm->size) {
    return false;
  }
  uint8_t bit_index = index % 8;

  // don't go across boundaries
  if (byte_index > bm->size) {
    return false;
  }

  uint8_t mask = 0b10000000 >> bit_index;

  return (bm->buffer[byte_index] & mask) ? true : false;
}

bool bitmapSetVal(BitMap *bm, uint64_t index, bool value) {
  uint64_t byte_index = index / 8;
  if (byte_index > bm->size) {
    return false;
  }
  uint8_t bit_index = index % 8;

  uint8_t mask = 0b10000000 >> bit_index;
  bm->buffer[byte_index] &= ~mask;

  if (value) {
    bm->buffer[byte_index] |= mask;
  }
  return true;
}
