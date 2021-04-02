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

#include <stdint.h>
#include <stddef.h>

#ifndef __cplusplus
// BOOL type, which is char
typedef char bool;
#define true 1
#define false 0
#endif

#endif // TYPE_H
