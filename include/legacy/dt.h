//===- dt.h - Descriptor tables ------------------------------------------===//
//
// This file contains basic definitions for GDT and IDT
//
//===---------------------------------------------------------------------===//

#ifndef PROTECT_H
#define PROTECT_H

#include "const.h"
#include "type.h"

/// The Global Descriptor
typedef struct s_descriptor {
  u16 limit_low;
  u16 base_low;
  u8 base_mid;
  u8 attr1;
  u8 limit_high_attr2;
  u8 base_high;
} DESCRIPTOR;

/// The interrupt descriptor for IA-32
/// Indicates where the ISR is located.
typedef struct s_gate {
  /// Lower 16 bits of the basee
  u16 base_low;

  /// Segment selector
  u16 sel;

  /// Unsed variable, set to 0
  u8 always0;

  /// Type and attributes, see docs
  u8 flags;

  /// Higher 16 bits of the base
  u16 base_high;
} GATE;

/// Function type for interrupt handler
typedef void (*int_handler)();

/// Index, the GDT entry index
///{
#define INDEX_DUMMY 0
#define INDEX_FLAT_C 1
#define INDEX_FLAR_RW 2
#define INDEX_VIDEO 3
// TSS
#define INDEX_TSS 4
#define INDEX_LDT_FIRST 5
///}

/// GDT Selector
///{
#define SELECTOR_DUMMY 0
#define SELECTOR_FLAT_C 0x08
#define SELECTOR_FLAT_RW 0x10
#define SELECTOR_VIDEO (0x18 + 3)
#define SELECTOR_TSS (0x20)
#define SELECTOR_LDT_FIRST (0x28)

#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS SELECTOR_VIDEO
///}

/// Each LDT has 2 entries
#define LDT_SIZE 2

/// Descriptor Types
///{

/// 32-bit segments
#define DA_32 0x4000

/// Guanularity: 4K
#define DA_LIMIT_4K 0x8000

/// DPL Definitions
///{
#define DA_DPL0 0x00
#define DA_DPL1 0x20
#define DA_DPL2 0x40
#define DA_DPL3 0x60
///}

/// Data segments attributes, defines flags and access bytes for the GDT
/// Access bytes defined as:
/// | Pr | Privl | S | Ex | DC | RW | AC |
/// 7....................................0
/// Pr: Present Bit
/// Privl: 2 bits, ring level
/// S: Descriptor type (Code, data, system)
/// Ex: Executable?
/// DC: Direction:  (For Data): 0 grows up, 1 grows down
///     Conforming: (For Code)
/// RW: Readable(For code) / Writable(For data)
/// Ac: Accessed, just set to 0
/// Gr: Granularity, 0 -> byte granularity, 1 -> 4KiB blocks
/// Sz: Size bit. 0 -> 16-bit protected mode, 1 -> 32 bit protected mode
///{

/// Present, Data, Read Only
#define DA_DR 0x90

/// Present, Data, RW
#define DA_DRW 0x92

/// Present, Data, Accessed, RW
#define DA_DRWA 0x93

/// Present, Code, Exec only
#define DA_C 0x98

/// Present, Code, Exec & Read
#define DA_CR 0x9A

/// Present, Code, Conforming, Exec only
#define DA_CCO 0x9C

/// Present, Code, Conforming, Exec & Read
#define DA_CCOR 0x9E
///}

/// System segments attributes, defines access bytes for the GDT.
/// For detailed definition, see docs for the Data segments.
///{

/// LDT
#define DA_LDT 0x82

/// Task gate
#define DA_TaskGate 0x85

/// 386 TSS
#define DA_386TSS 0x89

/// 386 Call Gate
#define DA_386CGate 0x8C

/// 386 Interrupt Gate
#define DA_386IGate 0x8E

/// 386 Trap Gate
#define DA_386TGate 0x8F
///}

/// Selector Attributes
///{
#define SA_RPL_MASK 0xFFFC
#define SA_RPL_0 0
#define SA_RPL_1 1
#define SA_RPL_2 2
#define SA_RPL_3 3

/// Task runs in RPL 1
#define SA_RPL_TASK SA_RPL_1
///}

#define SA_TI_MASK 0xFFFB
#define SA_TIG 0
#define SA_TIL 4

/// Interrupt Vectors
///{
#define INT_VECTOR_DIVIDE 0x0
#define INT_VECTOR_DEBUG 0x1
#define INT_VECTOR_NMI 0x2
#define INT_VECTOR_BREAKPOINT 0x3
#define INT_VECTOR_OVERFLOW 0x4
#define INT_VECTOR_BOUNDS 0x5
#define INT_VECTOR_INVAL_OP 0x6
#define INT_VECTOR_COPROC_NOT 0x7
#define INT_VECTOR_DOUBLE_FAULT 0x8
#define INT_VECTOR_COPROC_SEG 0x9
#define INT_VECTOR_INVAL_TSS 0xA
#define INT_VECTOR_SEG_NOT 0xB
#define INT_VECTOR_STACK_FAULT 0xC
#define INT_VECTOR_PROTECTION 0xD
#define INT_VECTOR_PAGE_FAULT 0xE
#define INT_VECTOR_COPROC_ERR 0x10

#define INT_VECTOR_IRQ0 0x20
#define INT_VECTOR_IRQ8 0x28
///}

/// TSS
///{
struct s_tss {
  u32 backlink;
  u32 esp0;
  u32 ss0;
  u32 esp1;
  u32 ss1;
  u32 esp2;
  u32 ss2;
  u32 cr3;
  u32 eip;
  u32 flags;
  u32 eax;
  u32 ecx;
  u32 edx;
  u32 ebx;
  u32 esp;
  u32 ebp;
  u32 esi;
  u32 edi;
  u32 es;
  u32 cs;
  u32 ss;
  u32 ds;
  u32 fs;
  u32 gs;
  u32 ldt;
  u16 trap;
  u16 iobase;
};

typedef struct s_tss TSS;
///}

/// This function is used to initialize IDT. It fills the specified function
/// to handle differnet interruptes. Note that this is an automated process,
/// so no arguments are passed to this function.
PUBLIC void init_prot(void);

/// Init a descriptor.
/// This function changes the global descriptor \p p_desc based on \p base,
/// \p limit, \p attribute.
PUBLIC void init_descriptor(DESCRIPTOR *p_desc, u32 base, u32 limit,
                            u16 attribute);

/// This function returns the physical address given the argument \p seg.
/// It looks up the GDT, gets and returns the base address.
PUBLIC u32 seg2phys(int seg);

/// This macro changes the virtual address to the physical address.
/// The virtual address 0 starts at the base of the segment it is located in.
#define vir2phys(seg_base, vir) (u32)(((u32)seg_base) + (u32)(vir))
#endif
