///===- idt.c - Interrupt Descriptor Table -------------------------------===///
/// Data structs related to interrupt (exception) handling
///===--------------------------------------------------------------------===///

#include "idt.h"
#include "fb.h"
#include "gdt.h"
#include "interrupts.h"
#include "io.h"
#include "mem.h"
#include "types.h"
#include "mouse.h"

static void remapPIC(int offset_1, int offset_2) {
  // The process:
  // 1. write ICW1 to 20H / A0H
  // 2. write ICW2 to 21H / A1H
  // 3. write ICW3 to A1H / A2H
  // 4. write ICW4 to 21H / A1H

  uint8_t a1, a2;
  a1 = inb(PIC1_DATA);
  io_wait();
  a2 = inb(PIC2_DATA);
  io_wait();

  // master, ICW1
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  // slave, ICW 1
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();

  // IRQ0 -> INT0x20, master, ICW2
  outb(PIC1_DATA, offset_1);
  io_wait();
  // IRQ8 -> INT0x28, master, ICW2
  outb(PIC2_DATA, offset_2);
  io_wait();

  // master, ICW3
  outb(PIC1_DATA, 0x04);
  io_wait();
  // slave, ICW3
  outb(PIC2_DATA, 0x02);
  io_wait();

  // mater, ICW4
  outb(PIC1_DATA, ICW4_8086);
  // slave, ICW4
  outb(PIC2_DATA, ICW4_8086);

  // master, clock only, disable others
  outb(PIC1_DATA, a1);
  io_wait();
  // slave, all int disabled
  outb(PIC2_DATA, a2);

  return;
}

void irqSetMask(unsigned char IRQline) {
  uint16_t port;
  uint8_t value;

  if (IRQline < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    IRQline -= 8;
  }
  value = inb(port) | (1 << IRQline);
  outb(port, value);
}

void irqClearMask(unsigned char IRQline) {
  uint16_t port;
  uint8_t value;

  if (IRQline < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    IRQline -= 8;
  }
  value = inb(port) & ~(1 << IRQline);
  outb(port, value);
}

void picSendEOI(int irq) {
  if (irq >= 8) {
    outb(PIC2_COMMAND, PIC_EOI);
  }
  outb(PIC1_COMMAND, PIC_EOI);
}

/// Set the offset field of the IDT descriptor (IDT Entry)
///
/// \param idt_desc The IDT Descriptor (IDT Entry)
/// \param offset The offset the set
static void idtDescSetOffset(IDTDesc *idt_desc, uint64_t offset) {
  idt_desc->offset_0 = (uint16_t)(offset & 0x000000000000ffff);
  idt_desc->offset_1 = (uint16_t)((offset & 0x00000000ffff0000) >> 16);
  idt_desc->offset_2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

/// Get the offset saved in current IDT Descriptor
///
/// \param idt_desc The IDT Descriptor
static uint64_t idtDescGetOffset(IDTDesc *idt_desc) {
  uint64_t offset = 0;

  offset |= (uint64_t)idt_desc->offset_0;
  offset |= (uint64_t)(idt_desc->offset_1) << 16;
  offset |= (uint64_t)(idt_desc->offset_2) << 32;

  return offset;
}

/// The content to be loadded into the IDTR (IDT Register)
static IDTR idtr;

/// Get the IDT Descriptor given the index (vector)
#define IDT_DESC_BY_INDEX(index)                                               \
  ((IDTDesc *)(idtr.base + index * sizeof(IDTDesc)))

/// Assign a interrupt handler function to the interrupt given the interrupt
/// vector
///
/// \param index The interrupt vector the handler to be assigned
/// \param handler The function pointer to a handler function
static void IntAssignFunc(uint64_t index,
                          void (*handler)(struct interrupt_frame *)) {
  IDTDesc *desc = IDT_DESC_BY_INDEX(index);
  idtDescSetOffset(desc, (uint64_t)handler);

  desc->selector = SEG_OFFSET_KERNEL_CODE;
  desc->type_attr = IDT_TA_Interrupt;
}

void prepareInterrupts() {
  asm("cli");
  remapPIC(PIC_MASTER_INT_START, PIC_SLAVE_INT_START);

  size_t idt_size = IDT_ENTRIES * sizeof(IDTDesc);
  if (idt_size > PAGE_SIZE) {
    printf("  %s: PANIC: For now we cannot allocate a continous region with "
           "more than one page: %d bytes\n",
           __func__, PAGE_SIZE);

    // Block here
    while (1) {
    }
  }

  idtr.limit = idt_size - 1;
  idtr.base = (uint64_t)requestPage();

  IntAssignFunc(EX_DOUBLE_FAULT, exHandlerDF);
  IntAssignFunc(EX_GENERAL_PROTECTION, exHandlerGP);
  IntAssignFunc(EX_PAGE_FAULT, exHandlerPF);

  asm("lidt %0" : : "m"(idtr));

  for (int i = 0; i < 16; i++) {
    irqSetMask(i);
  }

  IntAssignFunc(PIC_GET_INT_VECTOR(IRQ_KEYBOARD_INT), intHandlerKB);
  IntAssignFunc(PIC_GET_INT_VECTOR(IRQ_PS2_MOUSE), intHandlePS2Mouse);
  PS2MouseInit();

  irqClearMask(IRQ_KEYBOARD_INT);
  irqClearMask(IRQ_CASCADE);
  irqClearMask(IRQ_PS2_MOUSE);

  asm("sti");
}
