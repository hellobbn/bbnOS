///===- gdt.h - Global Descriptor ----------------------------------------===///
/// Definition of the global descriptor table (GDT)
///===--------------------------------------------------------------------===///
///
/// The global descriptor table is mainly used in the IA32 Compatibility Mode
/// instead of the Intel 64 Mode where most of the registers are ignored. This
/// file defines normal data structures for GDT, as described in Intel's Manual
/// The gdtdesc in Intel 64 Mode might be different from IA32 Mode, where the
/// size of the offset is 64-bit mode.
///
/// Reference: https://wiki.osdev.org/GDT
///
///===--------------------------------------------------------------------===///

#ifndef GDT_H
#define GDT_H

#include "types.h"

typedef struct GDTDesc {
  uint16_t size;
  uint64_t offset;
} __attribute__((packed)) GDTDesc;

typedef struct GDTEntry {
  uint16_t limite0;
  uint16_t base0;
  uint8_t base1;
  uint8_t access_byte;
  uint8_t limit1_flags;
  uint8_t base2;
} __attribute__((packed)) GDTEntry;

typedef struct GDT {
  // offset: 0x00
  GDTEntry NullEntry;

  // offset: 0x08
  GDTEntry KernelCode;

  // offset: 0x10
  GDTEntry KernelData;

  // offset: 0x18
  GDTEntry UserNull;

  // offset: 0x20
  GDTEntry UserCode;

  // offset: 0x28
  GDTEntry UserData;
} __attribute__((packed)) __attribute__((aligned(0x1000))) GDT;

extern GDT DefaultGDT;

extern void LoadGDT(GDTDesc *gdt_desc);

#endif // GDT_H