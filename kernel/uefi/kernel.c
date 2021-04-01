typedef unsigned long long size_t;

#include "boot.h"
#include "efi_mem.h"
#include "fb.h"
#include "types.h"

void kmain(BootInfo *boot_info) {
  // Drawing Pixels in framebuffer
  initFb(boot_info->framebuffer, boot_info->psf1_font);

  clearScreen();
  printf("==> Memory Map:\n");
  {
    uint64_t mmap_entries = boot_info->mmap_size / boot_info->mmap_desc_size;
    void *mmap = boot_info->mmap;
    for (size_t i = 0; i < mmap_entries; i++) {
      EFI_MEMORY_DESCRIPTOR *desc =
          (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap +
                                    (i * boot_info->mmap_desc_size));
      printf("%s ", EFI_MEMORY_TYPE_STRINGS[desc->type]);
      setColor(0xffff00ff);
      printf(" %d KB\t", (int)(desc->num_pages * 4096 / 1024));
      setColor(0xffffffff);

      if (i != 0 && i % 5 == 0) {
        printf("\n");
      }
    }
  }
  return;
}
