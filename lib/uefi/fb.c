///===- fb.c: Framebuffer operations ------------------------------------===///
/// Framebuffer Implementation
///===-------------------------------------------------------------------===///

#include "fb.h"
#include "types.h"

static Framebuffer *fb;
static PSF1_FONT *ft;

int initFb(Framebuffer *in_fb, PSF1_FONT *in_ft) {
  fb = in_fb;
  ft = in_ft;

  return FB_OP_SUCCESS;
}

int setFB(Framebuffer *in_fb) {
  fb = in_fb;

  return FB_OP_SUCCESS;
}

int setFont(PSF1_FONT *in_ft) {
  ft = in_ft;

  return FB_OP_SUCCESS;
}

void fbPutChar(unsigned int color, char chr, unsigned int x, unsigned int y) {
  unsigned int *pixel_ptr = (unsigned int *)fb->BaseAddress;
  char *font_ptr =
      ft->glyph_buffer + (chr * ft->psf1_header->charsize);

  // For a 16-byte glyph char, the char is "16x8"
  for (unsigned long y_off = y; y_off < y + 16; y_off++) {
    for (unsigned long x_off = x; x_off < x + 8; x_off++) {
      // This checks if the current position should be "1" (a light pixel)
      if ((*font_ptr & (0b10000000 >> (x_off - x))) > 0) {
        *(unsigned int *)(pixel_ptr + x_off +
                          (y_off * fb->PixelsPerScanline)) = color;
      }
    }
    // After each line, go ahead
    font_ptr++;
  }
}
