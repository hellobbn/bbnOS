#include "const.h"
#include "protect.h"
#include "type.h"
#include "fb.h"

PUBLIC u8 gdt_ptr[6];
PUBLIC DESCRIPTOR gdt[GDT_SIZE];

/** k_start_msg:
 *  print the kernel start message.
 */
PUBLIC void k_start_msg(void) {
    fb_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
             "- Kernel is loaded, now initializing....\n");
}

/** cstart:
 *  Copy the gdt from loader to kernel.
 *  The GDT is saved in the global variable `gdt` and the kernel's GDT pointer is
 *  in gdt_ptr.
 */
PUBLIC void cstart(void) {
    fb_print("- [cstart] Reloading GDT\n");

    // move GDT in loader to the new GDT
    memcpy(&gdt, // the dest gdt
           (void *)(*((
               u32 *)(&gdt_ptr[2]))), // get address of old gdt from old gdt_ptr
           *((u16 *)(&gdt_ptr[0])) + 1); // the size of gdt

    u16 *p_gdt_limit = (u16 *)(&gdt_ptr[0]);
    u32 *p_gdt_base = (u32 *)(&gdt_ptr[2]);
    *p_gdt_limit =
        GDT_SIZE * sizeof(DESCRIPTOR) - 1; // the limit position in gdt_ptr
    *p_gdt_base = (u32)&gdt;               // the base of gdt_ptr
    fb_print("- [cstart] GDT now reloaded\n");
}