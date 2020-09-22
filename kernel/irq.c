/* Initialize 8259A */
#include "const.h"
#include "fb.h"
#include "global.h"
#include "interrupt.h"
#include "io.h"
#include "thread.h"

PUBLIC void init_8259A(void) {
  // TODO: use macro for ports, master command: 0x20, master data: 0x21
  // remap the PIC: 8259A, master: 20H, 21H, slave: A0H, A1H
  // the process:
  // 1. write ICW1 to 20H / A0H
  // 2. write ICW2 to 21H / A1H
  // 3. write ICW3 to A1H / A2H
  // 4. write ICW4 to 21H / A1H

  // master, ICW1
  outb(0x20, 0x11);
  // slave, ICW 1
  outb(0xA0, 0x11);
  // IRQ0 -> INT0x20, master, ICW2
  outb(0x21, 0x20);
  // IRQ8 -> INT0x28, master, ICW2
  outb(0xA1, 0x28);
  // master, ICW3
  outb(0x21, 0x04);
  // slave, ICW3
  outb(0xA1, 0x02);
  // mater, ICW4
  outb(0x21, 0x01);
  // slave, ICW4
  outb(0xA1, 0x01);
  // master, clock only, disable others
  outb(0x21, 0xFE);
  // slave, all int disabled
  outb(0xA1, 0xFF);

  // now init irq table
  for (int i = 0; i < NUM_IRQ; ++i) {
    register_handler(spurious_irq, i);
  }

  return;
}

PUBLIC void spurious_irq(int irq) {
  // Call the requested IRQ
  irq_table[irq](irq);
}

PUBLIC void common_irq(int irq) {
  print("Spurious IRQ: ");
  fb_print_hex(irq);
  print("\n");
}

PUBLIC void clock_handler(int irq) {
  print("#");
  fb_print_dec(irq);

  if (clock_int_enter_time != 0) {
    print("!");
    return;
  }

  // switch task
  p_proc_ready++;
  if (p_proc_ready >= proc_table + MAX_THREAD) {
    p_proc_ready = proc_table;
  }
}

void register_handler(irq_handler handler, int irq) {
  // Disable irq before registering.
  // FIXME: should we re-enable it?
  disable_irq(irq);
  irq_table[irq] = handler;
}
