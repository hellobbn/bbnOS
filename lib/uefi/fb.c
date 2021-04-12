///===- fb.c: Framebuffer operations -------------------------------------===///
/// Framebuffer Implementation
///===--------------------------------------------------------------------===///
//
// TODO: Implement cursor move function
// TODO: Implement a bitmap indicating which character has been rendered
//
//===---------------------------------------------------------------------===///

#include "fb.h"
#include "types.h"

static Framebuffer *fb = NULL;
static PSF1_FONT *ft = NULL;
static uint64_t Color = 0xffffffff;
static CursorPosition cursor_position = {
    .cursor_X = 0,
    .cursor_Y = 0,
};

int fbInit(Framebuffer *in_fb, PSF1_FONT *in_ft, uint64_t color) {
  fb = in_fb;
  ft = in_ft;
  Color = color;

  cursor_position.cursor_X = 0;
  cursor_position.cursor_Y = 0;

  return FB_OP_SUCCESS;
}

int fbSetFb(Framebuffer *in_fb) {
  fb = in_fb;

  return FB_OP_SUCCESS;
}

int fbSetFont(PSF1_FONT *in_ft) {
  ft = in_ft;

  return FB_OP_SUCCESS;
}

void fbSetColor(uint64_t color) { Color = color; }

void _setColorOfPixel(uint64_t color, unsigned int x, unsigned int y) {
  *(unsigned int *)((unsigned int *)fb->BaseAddress + x +
                    (y * fb->PixelsPerScanline)) = color;
}

void _fbPutChar(uint64_t color, char chr, unsigned int x, unsigned int y) {
  if (!fb || !ft) {
    return;
  }

  char *font_ptr = ft->glyph_buffer + (chr * ft->psf1_header->charsize);

  // For a 16-byte glyph char, the char is "16x8"
  for (unsigned long y_off = y; y_off < y + 16; y_off++) {
    for (unsigned long x_off = x; x_off < x + 8; x_off++) {
      // This checks if the current position should be "1" (a light pixel)
      if ((*font_ptr & (0b10000000 >> (x_off - x))) > 0) {
        _setColorOfPixel(color, x_off, y_off);
      }
    }
    // After each line, go ahead
    font_ptr++;
  }
}

#define fbPutChar(c, x, y) (_fbPutChar(Color, c, x, y))

void fbResetCursor(size_t x, size_t y) {
  cursor_position.cursor_X = x;
  cursor_position.cursor_Y = y;
}

// BUG: This implementation seems to have bugs.
void fb_putchar(char c) {
  if (!fb || !ft) {
    return;
  }
  size_t x_pos = cursor_position.cursor_X;
  size_t y_pos = cursor_position.cursor_Y;

  // If secondly prints, clear first.
  // If this byte is set, the next char printed will clear the line
  static int clear = 0;

  if (x_pos > fb->Width || y_pos > fb->Height - 16) {
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
      for (unsigned int curr_y_pos = y_pos; curr_y_pos < y_pos + 16;
           curr_y_pos++) {
        for (x_pos = 0; x_pos < fb->Width; x_pos++) {
          _setColorOfPixel(0xff000000, x_pos, curr_y_pos);
        }
      }
      x_pos = 0;
    }
  } else if (c >= ' ') {
    fbPutChar(c, x_pos, y_pos);
    x_pos += 8;
  }

  if (x_pos >= fb->Width) {
    x_pos = 0;
    y_pos += 16;
    if (clear == 1) {
      for (unsigned int curr_y_pos = y_pos; curr_y_pos < y_pos + 16;
           curr_y_pos++) {
        for (x_pos = 0; x_pos < fb->Width; x_pos++) {
          _setColorOfPixel(0xff000000, x_pos, curr_y_pos);
        }
      }
      x_pos = 0;
    }
  }

  if (y_pos >= fb->Height - 16) {
    x_pos = 0;
    y_pos = 0;
    clear = 1;

    // do a clear now
    for (unsigned int curr_y_pos = y_pos; curr_y_pos < 16; curr_y_pos++) {
      for (x_pos = 0; x_pos < fb->Width; x_pos++) {
        _setColorOfPixel(0xff000000, x_pos, curr_y_pos);
      }
    }
    x_pos = 0;
  }

  cursor_position.cursor_X = x_pos;
  cursor_position.cursor_Y = y_pos;
}

void fbClearChar() {
  if (cursor_position.cursor_X < 8) {
    if (cursor_position.cursor_Y < 16) {
      return;
    }
    cursor_position.cursor_X = fb->Width - 8;
    cursor_position.cursor_Y -= 16;
  } else {
    cursor_position.cursor_X -= 8;
  }

  for (unsigned long y_off = cursor_position.cursor_Y;
       y_off < cursor_position.cursor_Y + 16; y_off++) {
    for (unsigned long x_off = cursor_position.cursor_X;
         x_off < cursor_position.cursor_X + 8; x_off++) {
      // This checks if the current position should be "1" (a light pixel)
      _setColorOfPixel(0, x_off, y_off);
    }
  }
}

void fbClearScreen(uint32_t color) {
  for (size_t y = 0; y < fb->Height; y++) {
    for (size_t x = 0; x < fb->PixelsPerScanline; x++) {
      _setColorOfPixel(color, x, y);
    }
  }
}
