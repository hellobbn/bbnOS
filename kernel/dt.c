/** dt.c
 *  initialize descriptor table
 */

#include "const.h"
#include "type.h"
#include "dt.h"
#include "global.h"

/** init_descriptor:
 *  initialize the descriptor given by p_desc
 * 
 *  @param p_desc   the given descriptor
 *  @param base     base address
 *  @param limit    limit size
 *  @param attribute the attribute of the table
 */
PUBLIC void init_descriptor(DESCRIPTOR *p_desc, u32 base, u32 limit, u16 attribute) {
    p_desc->limit_low = limit & 0xFFFF;
    p_desc->base_low = base & 0xFFFF;
    p_desc->base_mid = (base >> 16) & 0xFF;
    p_desc->attr1 = attribute & 0xFF;
    p_desc->limit_high_attr2 = ((limit >> 16) & 0x0F) | ((attribute >> 8) & 0xF0);
    p_desc->base_high = (base >> 24) & 0xFF;
}

/** seg2phys
 *  calculate physics address from segment
 *
 *  @param seg  the selector of the segment
 */
PUBLIC u32 seg2phys(int seg) {
    DESCRIPTOR* p_dest = &gdt[seg >> 3];    // seg >> 3: the index of the seg
    return ((p_dest->base_high << 24) | (p_dest->base_mid << 16) | p_dest->base_low);
}