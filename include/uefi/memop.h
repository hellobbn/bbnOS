///===- memop.h - Generic memory operations -----------------------------===///
/// Basic and generic memory operations
///===-------------------------------------------------------------------===///
///
/// This file defines basec and generic operations on memory, and most of them
/// are NOT memory safe. This *might* be faster than normal `for` or `while`
/// operations but it is not a guarantee.
///
///===-------------------------------------------------------------------===///

#ifndef MEMOP_H
#define MEMOP_H

#include "types.h"

/// Set a region of memory to a specific value
///
/// \param d The region of memory
/// \param s The value to be set
/// \param c The size of the region
/// \return The head of the memory region (usually the same as parameter d)
void *memset(void *d, int s, size_t c);

#endif // MEMOP_H
