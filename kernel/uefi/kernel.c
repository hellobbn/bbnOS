#include "bitmap.h"
#include "boot.h"
#include "efi_mem.h"
#include "fb.h"
#include "mem.h"
#include "types.h"

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

void kmain(BootInfo *boot_info) {
  // Drawing Pixels in framebuffer
  initFb(boot_info->framebuffer, boot_info->psf1_font, 0xffffffff);
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

  printf("%s/mem: Reserve Kernel address space: %08lX - %08lX\n",
         __func__, (uint64_t)&_KernelStart, (uint64_t)&_KernelEnd);
  lockPages(&_KernelStart, kernel_pages);

  printf("==> %s: Summary: \n", __func__);
  printf("Free: %lu KB \nUsed: %lu KB \nReserved: %lu KB\n",
         getFreeMemSize() / 1024, getUsedMemSize() / 1024,
         getReservedMemSize() / 1024);

  for (int t = 0; t < 20; t++) {
    void *address = requestPage();
    printf("TEST: %lX\n", (uint64_t)address);
  }

  while (1) {
  }
  return;
}
