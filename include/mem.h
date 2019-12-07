/* Memory Operation Header */
#ifndef MEM_H
#define MEM_H

#include "const.h"

/** memcpy:
 *  copy memory from one region to another.
 *  Implemented in lib/mem.asm
 * 
 *  @param pDest  The dest memory pointer
 *  @param pSrc   The source memory pointer
 *  @param iSize  The size (in bytes) to be copied
 */
PUBLIC void *memcpy(void *pDest, void *pSrc, int iSize);

#endif