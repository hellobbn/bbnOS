/* Desciptor Table */
#ifndef DT_H
#define DT_H

#include "const.h"
#include "protect.h"
#include "type.h"

PUBLIC u8 gdt_ptr[6];
PUBLIC DESCRIPTOR gdt[GDT_SIZE];
PUBLIC u8 idt_ptr[6];
PUBLIC GATE idt[IDT_SIZE];

PUBLIC void init_prot(void); // initialzes IDT

#endif