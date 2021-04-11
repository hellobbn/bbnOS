#include "bitmap.h"
#include "boot.h"
#include "efi_mem.h"
#include "fb.h"
#include "gdt.h"
#include "mem.h"
#include "memop.h"
#include "types.h"
#include "idt.h"

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

void kmain(BootInfo *boot_info) {
  fbInit(boot_info->framebuffer, boot_info->psf1_font, 0xffffffff);

  // Set up GDT
  printf("==> %s: Setting Up GDT\n", __func__);
  GDTDesc gdt_desc;
  gdt_desc.size = sizeof(GDT) - 1;
  gdt_desc.offset = (uint64_t)&DefaultGDT;
  LoadGDT(&gdt_desc);
  printf("<== %s: GDT Successfully loaded\n", __func__);

  // Clear screen on demand
  // clearScreen();

  // Get total memory
  printf("==> %s: TOTAL_MEM: %lu\n", __func__,
         getMemSize(boot_info->mmap,
                    boot_info->mmap_size / boot_info->mmap_desc_size,
                    boot_info->mmap_desc_size));

  readEFIMemoryMap(boot_info->mmap, boot_info->mmap_size,
                   boot_info->mmap_desc_size);

  uint64_t kernel_size = (uint64_t)(&_KernelEnd) - (uint64_t)(&_KernelStart);
  uint64_t kernel_pages = (uint64_t)(kernel_size / 4096 + 1);

  printf("<== %s/mem: Reserve Kernel address space: %08lX - %08lX\n", __func__,
         (uint64_t)&_KernelStart, (uint64_t)&_KernelEnd);
  lockPages(&_KernelStart, kernel_pages);

  printf("==> %s/mem: Summary: \n", __func__);
  printf("    Free: %lu KB \n    Used: %lu KB \n    Reserved: %lu KB\n",
         getFreeMemSize() / 1024, getUsedMemSize() / 1024,
         getReservedMemSize() / 1024);

  printf("==> %s/mem: Setting up paging\n", __func__);
  PageTable *pml4 = (PageTable *)requestPage();
  memset((void *)pml4, 0, 0x1000);
  PageTableManager ptm;
  initPageTableManager(&ptm, pml4);

  for (uint64_t t = 0;
       t < getMemSize(boot_info->mmap,
                      boot_info->mmap_size / boot_info->mmap_desc_size,
                      boot_info->mmap_desc_size);
       t += 0x1000) {
    pageTableManagerMapMemory(&ptm, (void *)t, (void *)t);
  }

  uint64_t fbBase = (uint64_t)boot_info->framebuffer->BaseAddress;
  uint64_t fbSize = (uint64_t)boot_info->framebuffer->BufferSize + 0x1000;
  lockPages((void *)fbBase, fbSize / 0x1000 + 1);
  for (uint64_t t = fbBase; t < fbBase + fbSize; t += 0x1000) {
    pageTableManagerMapMemory(&ptm, (void *)t, (void *)t);
  }

  asm("mov %0, %%cr3" : : "r"(pml4));

  printf("<== %s/mem: Paging set up ^_^\n", __func__);

  printf("==> %s:/int: Prapare Interrupts...\n", __func__);
  prepareInterrupts();
  printf("<== %s/int: Interrupts Set up.\n", __func__);

  while (1) {
  }
}
