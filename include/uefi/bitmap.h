///===- bitmap.h - BitMap Implementation --------------------------------===///
/// The kernel's implementation of bitmap
///===-------------------------------------------------------------------===///
///
/// The bitmap is used to save space and provide high performance for kernel
/// parts like paging.
///
///===-------------------------------------------------------------------===///

#include "types.h"

/// The BitMap type
typedef struct BitMap {
  /// The size of the bitmap, in bytes, a byte = 8 bits
  size_t size;

  /// The bitmap buffer
  uint8_t *buffer;
} BitMap;

/// Operator Offload: load the value at the given index
bool bitmapGetVal(BitMap *bm, uint64_t index);

/// Set the value of the bitmap at a given index
void bitmapSetVal(BitMap *bm, uint64_t index, bool value);
