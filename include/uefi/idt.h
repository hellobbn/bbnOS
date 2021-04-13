///===- idt.h - Interrupt Descriptor Table -------------------------------===///
/// Data structs related to interrupt (exception) handling
///===--------------------------------------------------------------------===///
///
/// An interrupt descriptor table is a table declaring the handlers for
/// interruptions and handlers, and it tells where the actual interrupt
/// handler is.
///
///===--------------------------------------------------------------------===///

#ifndef IDT_H
#define IDT_H

#include "types.h"

/// Number of entries in IDT
#define IDT_ENTRIES 256

/// IDT type_addr attribute definietion
/// | P | DPL | S | Type |
/// 0   1     3   4      7
///{
#define IDT_TA_Interrupt 0b10001110
#define IDT_TA_CallGate 0b10001100
#define IDT_TA_TrapGate 0b10001111
///}

/// Interrupt Descriptor for Intel 64 Mode (Not for compatible mode)
typedef struct IDTDesc {
  /// Offset 15...0
  uint16_t offset_0;

  /// Segment selector for the destination code segment
  uint16_t selector;

  /// Bit[0..1]: ist: interrupt stack table
  uint8_t ist;

  /// type and attributes
  uint8_t type_attr;

  /// Offset 16...31
  uint16_t offset_1;

  /// Offset 32...63
  uint32_t offset_2;

  /// Reserved
  uint32_t reserved_field;
} __attribute__((packed)) IDTDesc;

/// The data structure to be loadded to the IDT register
typedef struct IDTR {
  uint16_t limit;
  uint64_t base;
} __attribute((packed)) IDTR;

/// Send EOI to parent/slave PIC
void picSendEOI(int irq);

/// IRQ set and clear mask
///{
void irqSetMask(unsigned char IRQline);
void irqClearMask(unsigned char IRQline);
///}

/// Definition of IRQs
enum IRQ_NUM {
  IRQ_PROGRAMMABLE_INT_TIMER_INT = 0,
  IRQ_KEYBOARD_INT = 1,
  IRQ_CASCADE = 2,
  IRQ_COM2 = 3,
  IRQ_COM1 = 4,
  IRQ_LPT2 = 5,
  IRQ_FLOPPY_DISK = 6,
  IRQ_LPT1 = 7,
  IRQ_CMOS_REALTIME_CLOCK = 8,
  IRQ_PERIPHERALS_LEGACYSCSI_NIC = 9,
  IRQ_PERIPHERALS_SCSI_NIC_1 = 10,
  IRQ_PERIPHERALS_SCSI_NIC_2 = 11,
  IRQ_PS2_MOUSE = 12,
  IRQ_FPU_COPROCESSOR_INTERPROCESSOR = 13,
  IRQ_PRIMARY_ATA_DISK = 14,
  IRQ_SECONDARY_ATA_DISK = 15,
};

/// PIC (8259) Definitions
///{
#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

#define PIC_EOI 0x20

#define PIC_MASTER_INT_START 0x20
#define PIC_SLAVE_INT_START 0x28

#define PIC_GET_INT_VECTOR(IRQ)                                                \
  (IRQ < 8 ? (PIC_MASTER_INT_START + IRQ) : (PIC_SLAVE_INT_START + IRQ - 8))
///}

/// Prepare the interrupt handlers. Assign each exception and (user defined)
/// interrupt the defined handler
void prepareInterrupts();
#endif // IDT_H
