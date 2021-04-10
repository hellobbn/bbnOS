///===- idt.c - Interrupt Descriptor Table -------------------------------===///
/// Data structs related to interrupt (exception) handling
///===--------------------------------------------------------------------===///

#include "idt.h"
#include "fb.h"
#include "gdt.h"
#include "interrupts.h"
#include "mem.h"
#include "types.h"

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

  IntAssignFunc(EX_PAGE_FAULT, exHandlerPF);

  asm("lidt %0" : : "m"(idtr));
}
