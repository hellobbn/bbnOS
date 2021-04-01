///===- types.h - Kernel types definition -------------------------------===///
/// Kernel normal types defnition. Explicitly declaring the size of type.
///===-------------------------------------------------------------------===///
///
/// This file declares the types that the kernel uses, and explicitly declaring
/// the size of each type. For example, the 32-bit int is called: `int32_t`,
/// and we also allow normal type declarations, like int_t, double_t, long_t,
/// when using specific sized types are not enforced.
///
///===-------------------------------------------------------------------===///

#ifndef TYPE_H
#define TYPE_H

typedef int int_t;
typedef long long_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;

// BOOL type, which is char
typedef char bool;
#define true 1
#define false 0

#endif // TYPE_H
