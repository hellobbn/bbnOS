/* Initialize 8259A */
#include "const.h"
#include "io.h"
#include "fb.h"
#include "thread.h"
#include "global.h"
#include "i8259.h"

/** init_8259A:
 *  remap the PIC so that the interrupt will not confilct
 *
 *  TODO: Use macro for the ports, master_command: 0x20, master_data: 0x21
 */
PUBLIC void init_8259A(void) {
    // remap the PIC: 8259A, master: 20H, 21H, slave: A0H, A1H
    // the process:
    // 1. write ICW1 to 20H / A0H
    // 2. write ICW2 to 21H / A1H
    // 3. write ICW3 to A1H / A2H
    // 4. write ICW4 to 21H / A1H
    outb(0x20, 0x11); // master 8259, ICW1
    outb(0xA0, 0x11); // slave 8259, ICW 1
    outb(0x21, 0x20); // IRQ0 -> INT0x20, master, ICW2
    outb(0xA1, 0x28); // IRQ8 -> INT0x28, slave, ICW2
    outb(0x21, 0x04); // master, ICW3
    outb(0xA1, 0x02); // slave, ICW3
    outb(0x21, 0x01); // master, ICW4
    outb(0xA1, 0x01); // slave, ICW4
    outb(0x21, 0xFF); // master - clock only
    outb(0xA1, 0xFF); // slave - All int disabled

    // now init irq table
    for(int i = 0; i < NUM_IRQ; ++ i) {
        register_handler(spurious_irq, i);
    }

    return;
}

/** spurious_irq:
 *  The common routine for IRQ
 */
PUBLIC void spurious_irq(int irq) {
    irq_table[irq](irq);        // CALL THE REQUESTED IRQ
}

/** common_irq:
 *  the common irq routine
 */
PUBLIC void common_irq(int irq) {
    print("Spurious IRQ: ");
    fb_print_hex(irq);
    print("\n");
}

/** clock_handler:
 *  handles IRQ0 -> the clock
 * 
 *  @note in oranges, it is moved to a file `clock.c`
 */
PUBLIC void clock_handler(int irq) {
    print("#");

    if(clock_int_enter_time != 0) {
        print("!");
        return;
    }

    // switch task
    p_proc_ready ++;
    if(p_proc_ready >= proc_table + MAX_THREAD) {
        p_proc_ready = proc_table;
    }
}

/** register_handler:
 *  register a function to a given IRQ
 * 
 * @param func  the IRQ handler
 * @param irq   the IRQ to register
 */
void register_handler(irq_handler handler, int irq) {
    // disable_irq here
    disable_irq(irq);
    irq_table[irq] = handler;
}