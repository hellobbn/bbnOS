/* Memory Operation Header */
#ifndef MEM_H
#define MEM_H

#include "const.h"

/// Copy memory from one region \p pSrc to the destination \p pDest
/// The bytes copied is specified by \p iSize
/// Implemented in lib/mem.asm
PUBLIC void *memcpy(void *pDest, void *pSrc, int iSize);

///  Set a region of memory \p pDest to a value \p ch.
/// The bytes set is specified by \p size.
PUBLIC void memset(void *pDest, char ch, int size);

/// Disable all interrupts.
PUBLIC void disable_int(void);

/// Enable all interruptes.
PUBLIC void enable_int(void);
#endif
