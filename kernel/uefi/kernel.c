typedef unsigned long long size_t;

#include "fb.h"

void kmain(Framebuffer *framebuffer, PSF1_FONT *psf1_font) {
  // Drawing Pixels in framebuffer
  initFb(framebuffer, psf1_font);
  printf("Printf test\n\n hello \t byebye \n 11 \t 123 \n %d \n %X", 123, 16);
  return;
}
