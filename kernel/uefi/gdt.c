///===- gdt.c - Global Descriptor ----------------------------------------===///
/// Definition of the global descriptor table (GDT)
///===--------------------------------------------------------------------===///
///
/// We use the flat memory model here, where every base in the entry is 0
///
///===--------------------------------------------------------------------===///

#include "gdt.h"

__attribute__((aligned(0x1000))) GDT DefaultGDT = {
    .NullEntry = {0, 0, 0, 0x00, 0x00, 0},
    .KernelCode = {0, 0, 0, 0x9a, 0xa0, 0},
    .KernelData = {0, 0, 0, 0x92, 0xa0, 0},
    .UserNull = {0, 0, 0, 0x00, 0x00, 0},
    .UserCode = {0, 0, 0, 0x9a, 0xa0, 0},
    .UserData = {0, 0, 0, 0x9a, 0xa0, 0},
};