typedef unsigned long long size_t;

#include "fb.h"

void kmain(Framebuffer *framebuffer, PSF1_FONT *psf1_font) {
  // Drawing Pixels in framebuffer
  initFb(framebuffer, psf1_font);
  fbPutChar(0xffffffff, 'G', 0, 0);
  return;
}
