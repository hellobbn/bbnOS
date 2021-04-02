#include "bitmap.h"
#include "boot.h"
#include "efi_mem.h"
#include "fb.h"
#include "mem.h"
#include "types.h"

void kmain(BootInfo *boot_info) {
  // Drawing Pixels in framebuffer
  initFb(boot_info->framebuffer, boot_info->psf1_font, 0xffffffff);

  clearScreen();

  // Get total memory
  printf("==> %s: TOTAL_MEM: %lu\n", __func__,
         getMemSize(boot_info->mmap,
                    boot_info->mmap_size / boot_info->mmap_desc_size,
                    boot_info->mmap_desc_size));

  readEFIMemoryMap(boot_info->mmap, boot_info->mmap_size,
                   boot_info->mmap_desc_size);

  printf("==> %s: Summary: \n", __func__);
  printf("Free: %lu KB \n Used: %lu KB \n Reserved: %lu KB\n", getFreeMemSize() / 1024,
         getUsedMemSize() / 1024, getReservedMemSize() / 1024);

  while (1) {
    
  }
  return;
}
