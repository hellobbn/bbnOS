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

/// Prepare the interrupt handlers. Assign each exception and (user defined)
/// interrupt the defined handler
void prepareInterrupts();

#endif // IDT_H
