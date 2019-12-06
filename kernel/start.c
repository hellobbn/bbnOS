#include "const.h"
#include "protect.h"
#include "type.h"
#include "fb.h"

PUBLIC void *memcpy(void *pDest, void *pSrc, int iSize);

PUBLIC void disp_str(char *pszInfo);

PUBLIC u8 gdt_ptr[6];
PUBLIC DESCRIPTOR gdt[GDT_SIZE];

PUBLIC void cstart() {
    fb_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
             "- Initializing kernel...\n");

    // move GDT in loader to the new GDT
    memcpy(&gdt, // the dest get
           (void *)(*((
               u32 *)(&gdt_ptr[2]))), // get address of old gdt from old gdt_ptr
           *((u16 *)(&gdt_ptr[0])) + 1); // the size of gdt

    u16 *p_gdt_limit = (u16 *)(&gdt_ptr[0]);
    u32 *p_gdt_base = (u32 *)(&gdt_ptr[2]);
    *p_gdt_limit =
        GDT_SIZE * sizeof(DESCRIPTOR) - 1; // the limit position in gdt_ptr
    *p_gdt_base = (u32)&gdt;               // the base of gdt_ptr
}