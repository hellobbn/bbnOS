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

/** memset:
 *  set a region of memory to a value
 * 
 *  @param p_dest   the start address
 *  @param ch       the value
 *  @param size     size of the region
 */
PUBLIC void memset(void* p_dest, char ch, int size);
#endif