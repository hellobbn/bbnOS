#include "const.h"
#include "dt.h"
#include "fb.h"
#include "mem.h"
#include "type.h"

/** k_start_msg:
 *  print the kernel start message.
 */
PUBLIC void k_start_msg(void) {
    print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
          "- Kernel is loaded, now initializing....\n");
}

/** cstart:
 *  Copy the gdt from loader to kernel.
 *  The GDT is saved in the global variable `gdt` and the kernel's GDT pointer
 * is in gdt_ptr.
 */
PUBLIC void cstart(void) {
    print("- [cstart] Reloading GDT\n");

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
    print("- [cstart] GDT now reloaded\n");

    // set the IDT
    print("- [cstart] Set up IDT\n");
    u16 *p_idt_limit = (u16 *)(&idt_ptr[0]); // 2 bytes for limit
    u32 *p_idt_base = (u32 *)(&idt_ptr[2]);  // 4 bytes for base
    *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
    *p_idt_base = (u32)(&idt);

    init_prot();
    print("- [cstart] IDT set up done\n");
}