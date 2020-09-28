#include "const.h"
#include "dt.h"
#include "fb.h"
#include "global.h"
#include "interrupt.h"
#include "mem.h"
#include "string.h"
#include "syscall.h"
#include "thread.h"
#include "time.h"
#include "type.h"

/// Printing the kernel startup message. Whoo!
PUBLIC void k_start_msg(void) {
  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "- Kernel is loaded, now initializing....\n");
}

/// Copy the gdt from loader to kernel.
/// Previously we have copied the GDT to the `gdt_ptr` in the kernel.asm using
/// `sgdt`.
PUBLIC void cstart(void) {
  printf("- [cstart] Reloading GDT\n");

  // Move GDT in loader to the new GDT, we get the address and the size of the
  // old GDT from the old `get_ptr`
  memcpy(&gdt, (void *)(*((u32 *)(&gdt_ptr[2]))), *((u16 *)(&gdt_ptr[0])) + 1);

  // Re-point the GDT to the new location.
  u16 *p_gdt_limit = (u16 *)(&gdt_ptr[0]);
  u32 *p_gdt_base = (u32 *)(&gdt_ptr[2]);
  *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
  *p_gdt_base = vir2phys(seg2phys(SELECTOR_KERNEL_DS), &gdt);
  printf("- [cstart] GDT now reloaded\n");

  // set the tss
  printf("- [cstart] Loading TSS to GDT\n");
  memset(&tss, 0, sizeof(TSS));
  tss.ss0 = SELECTOR_KERNEL_DS;
  init_descriptor(&gdt[INDEX_TSS], vir2phys(seg2phys(SELECTOR_KERNEL_DS), &tss),
                  sizeof(TSS) - 1, DA_386TSS);
  tss.iobase = sizeof(TSS);
  printf("- [cstart] Load TSS Done\n");

  // set the LDT
  printf("- [cstart] Loading LDT to GDT\n");
  PROCESS *p_proc = proc_table;
  u16 selector_ldt = INDEX_LDT_FIRST << 3;
  for (int i = 0; i < MAX_THREAD; ++i) {
    init_descriptor(&gdt[selector_ldt >> 3],
                    vir2phys(seg2phys(SELECTOR_KERNEL_DS), proc_table[i].ldts),
                    LDT_SIZE * sizeof(DESCRIPTOR) - 1, DA_LDT);
    p_proc++;
    selector_ldt += 1 << 3;
  }
  printf("- [cstart] Load LDT done\n");

  // finally, set the IDT
  printf("- [cstart] Set up IDT\n");
  u16 *p_idt_limit = (u16 *)(&idt_ptr[0]);
  u32 *p_idt_base = (u32 *)(&idt_ptr[2]);
  *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
  *p_idt_base = (u32)(&idt);

  init_prot();
  printf("- [cstart] IDT set up done\n");
}

///  A function to test the thread implementation,
/// It prints the char 'A' in an infinite loop
///
PUBLIC void testA() {
  while (1) {
    // nothing here
    printf("- [testA] test A ");
    delay(1);
  }
}

PUBLIC void testB() {
  while (1) {
    // print 'B'
    test_call();
    printf("- [testB] test B ");
    delay(1);
  }
}

PUBLIC void testC() {
  while (1) {
    // print 'C'
    printf("- [testC] test C ");
    delay(1);
  }
}

/// The main kernel function
int kmain() {
  printf("- [kmain] start here\n");
  disable_int();

  // enable clock IRQ
  // TODO: we need to define IRQs
  register_handler(clock_handler, 0);
  register_handler(keyboard_handler, 1);
  // enable_irq(0);
  enable_irq(1);

  // Initialize tasks
  TASK *p_task = task_table;
  PROCESS *p_proc = proc_table;
  char *p_task_stack = task_stack;
  u16 selector_ldt = SELECTOR_LDT_FIRST;

  for (int i = 0; i < MAX_THREAD; ++i) {
    // fill in PCB
    strcpy(p_proc->p_name, p_task->name);
    p_proc->pid = i;

    p_proc->ldt_sel = selector_ldt;

    memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
    p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
    memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
    p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
    p_proc->registers.cs =
        ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | SA_RPL_TASK;

    u32 tmp = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | SA_RPL_TASK;
    p_proc->registers.ds = tmp;
    p_proc->registers.es = tmp;
    p_proc->registers.fs = tmp;
    p_proc->registers.ss = tmp;
    p_proc->registers.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | SA_RPL_TASK;

    p_proc->registers.eip = (u32)p_task->initial_eip;
    p_proc->registers.esp = (u32)p_task_stack + (u32)p_task->stack_size - 1;
    p_proc->registers.eflags = 0x1202;

    p_task_stack += p_task->stack_size;
    p_task++;
    p_proc++;
    selector_ldt += 1 << 3;
  }
  printf("- [kmain] trying to switch to tasks\n");
  p_proc_ready = proc_table;
  // set int enter time
  clock_int_enter_time = 0;
  // delay(1);
  restart();
  while (1) {
    // do nothing
  }
}
