///===- fb.c: Framebuffer operations ------------------------------------===///
/// Framebuffer Implementation
///===-------------------------------------------------------------------===///

#include "fb.h"
#include "types.h"

static Framebuffer *fb = NULL;
static PSF1_FONT *ft = NULL;

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
  if (!fb || !ft) {
    return;
  }

  unsigned int *pixel_ptr = (unsigned int *)fb->BaseAddress;
  char *font_ptr = ft->glyph_buffer + (chr * ft->psf1_header->charsize);

  // For a 16-byte glyph char, the char is "16x8"
  for (unsigned long y_off = y; y_off < y + 16; y_off++) {
    for (unsigned long x_off = x; x_off < x + 8; x_off++) {
      // This checks if the current position should be "1" (a light pixel)
      if ((*font_ptr & (0b10000000 >> (x_off - x))) > 0) {
        *(unsigned int *)(pixel_ptr + x_off + (y_off * fb->PixelsPerScanline)) =
            color;
      }
    }
    // After each line, go ahead
    font_ptr++;
  }
}

void fb_putchar(char c) {
  if (!fb || !ft) {
    return;
  }
  static unsigned int x_pos = 0;
  static unsigned int y_pos = 0;

  // If secondly prints, clear first.
  // If this byte is set, the next char printed will clear the line
  static int clear = 0;

  if (x_pos > fb->Width || y_pos > fb->Height) {
    // This Shouldn't happen (BUG)
    return;
  }

  if (c == 0x08 && x_pos) {
    x_pos -= 8;
  } else if (c == 0x09) {
    x_pos = (x_pos + 64) & ~(64 - 1);
  } else if (c == '\r') {
    x_pos = 0;
  } else if (c == '\n') {
    x_pos = 0;
    y_pos += 16;
    if (clear == 1) {
      for (x_pos = 0; x_pos < fb->Width; x_pos += 8) {
        fbPutChar(0, ' ', x_pos, y_pos);
      }
      x_pos = 0;
    }
  } else if (c >= ' ') {
    fbPutChar(0xffffffff, c, x_pos, y_pos);
    x_pos += 8;
  }

  if (x_pos >= fb->Width) {
    x_pos = 0;
    y_pos += 8;
  }

  if (y_pos >= fb->Height) {
    x_pos = 0;
    y_pos = 0;
    clear = 1;

    // do a clear now
    for (x_pos = 0; x_pos < fb->Width; x_pos++) {
      fbPutChar(0, ' ', x_pos, y_pos);
    }
    x_pos = 0;
  }
}
