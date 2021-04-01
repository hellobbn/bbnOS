///===- boot.h - BootInfo holder ---------------------------------------===///
/// This file defines structures describing the the boot related info.
///===------------------------------------------------------------------===///
///
/// The bootloader uses this function to pass the descriptors of framebuffer,
/// font, memory map etc to the kernel so that the kernel can successfully set
/// up various services.
///===------------------------------------------------------------------===///

#ifndef BOOT_H
#define BOOT_H

#include "fb.h"
#include "types.h"

typedef struct {
  Framebuffer *framebuffer;
  PSF1_FONT *psf1_font;
  void *mmap;
  uint64_t mmap_size;
  uint64_t mmap_desc_size;
} BootInfo;

#endif
