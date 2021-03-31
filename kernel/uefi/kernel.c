typedef unsigned long long size_t;
typedef struct {
  void *BaseAddress;
  size_t BufferSize;
  unsigned int Width;
  unsigned int Height;
  unsigned int PixelsPerScanline;
} Framebuffer;

/// PSF1 Font Related structures and Macros
///{
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
typedef struct {
  unsigned char magic[2];
  unsigned char mode;
  unsigned char charsize;
} PSF1_HEADER;

typedef struct {
  PSF1_HEADER *psf1_header;
  void *glyph_buffer;
} PSF1_FONT;
///}

void put_char(Framebuffer *framebuffer, PSF1_FONT *psf1_font,
              unsigned int color, char chr, unsigned int x, unsigned int y) {
  unsigned int *pixel_ptr = (unsigned int *)framebuffer->BaseAddress;
  char *font_ptr =
      psf1_font->glyph_buffer + (chr * psf1_font->psf1_header->charsize);

  // For a 16-byte glyph char, the char is "16x8"
  for (unsigned long y_off = y; y_off < y + 16; y_off++) {
    for (unsigned long x_off = x; x_off < x + 8; x_off++) {
      // This checks if the current position should be "1" (a light pixel)
      if ((*font_ptr & (0b10000000 >> (x_off - x))) > 0) {
        *(unsigned int *)(pixel_ptr + x_off +
                          (y_off * framebuffer->PixelsPerScanline)) = color;
      }
    }
    // After each line, go ahead
    font_ptr++;
  }
}

void kmain(Framebuffer *framebuffer, PSF1_FONT *psf1_font) {
  // Drawing Pixels in framebuffer
  put_char(framebuffer, psf1_font, 0xffffffff, 'G', 10, 10);
  return;
}
