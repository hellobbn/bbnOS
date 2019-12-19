#include "const.h"
#include "dt.h"
#include "fb.h"
#include "mem.h"
#include "thread.h"
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

    // set the tss
    print("- [cstart] Loading TSS to GDT\n");
    memset(&tss, 0, sizeof(TSS));
    tss.ss0 = SELECTOR_KERNEL_DS;
    init_descriptor(&gdt[INDEX_TSS],
                    vir2phys(seg2phys(SELECTOR_KERNEL_DS), &tss),
                    sizeof(TSS) - 1, DA_386TSS);
    tss.iobase = sizeof(TSS);
    print("- [cstart] Load TSS Done\n");

    // set the LDT
    print("- [cstart] Loading LDT to GDT\n");
    init_descriptor(&gdt[INDEX_LDT_FIRST],
                    vir2phys(seg2phys(SELECTOR_KERNEL_DS),
                             proc_table[0].ldts), // base address
                    LDT_SIZE * sizeof(DESCRIPTOR) - 1, DA_LDT);
    print("- [cstart] Load LDT done\n");
}

/** testA:
 *  A function to test the thread implementation,
 *  It prints the char 'A' in an infinite loop
 */
void testA() {
    while (1) {
        // nothing here
        print("- [testA] in test A now.\n");
        delay(100);
    }
}

/** kmain:
 *  The kernel main function
 */
int kmain() {
    print("- [kmain] start here\n");

    // Initialize LDT
    PROCESS *p_proc = proc_table;

    p_proc->ldt_sel = SELECTOR_LDT_FIRST;
    memcpy(&p_proc->ldts[0],
           &gdt[SELECTOR_KERNEL_CS >> 3] // SELECTOR >> 3 is actually the CS in
                                         // gdt it copies the CS GDT to LDT
           ,
           sizeof(DESCRIPTOR)); // the first descriptor entry

    p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5; // change the DPL
    memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
           sizeof(DESCRIPTOR));                           // The DS
    p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5; // change the DPL

    p_proc->registers.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL |
                           SA_RPL_TASK; // 0 -> the first selector of LDT
    p_proc->registers.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL |
                           SA_RPL_TASK; // 8 -> the selector for ds
    u32 tmp_ds = p_proc->registers.ds;
    p_proc->registers.es = tmp_ds;
    p_proc->registers.fs = tmp_ds;
    p_proc->registers.ss = tmp_ds;
    p_proc->registers.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | SA_RPL_TASK;
    p_proc->registers.eip = (u32)testA; // points to the task
    p_proc->registers.esp = (u32)task_stack + STACK_SIZE_TOTAL;
    p_proc->registers.eflags = 0x1202; // IF = 1, IOPL = 1, bit 2 is always 1

    p_proc_ready = proc_table;
    print("- [kmain] trying to switch to task A\n");
    delay(1);
    restart();
    while (1) {
        // do nothing
    }
}