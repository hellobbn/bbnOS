typedef unsigned long long size_t;
typedef struct {
  void *BaseAddress;
  size_t BufferSize;
  unsigned int Width;
  unsigned int Height;
  unsigned int PixelsPerScanline;
} Framebuffer;

void kmain(Framebuffer *framebuffer) {

  // Drawing Pixels in framebuffer
  {
    unsigned int y = 50;
    // Bytes per pixel
    unsigned int BBP = 4;
    for (unsigned int x = 0; x < (framebuffer->Width / 2 * BBP); x++) {
      *(unsigned int *)(x + (y * framebuffer->PixelsPerScanline * BBP) +
                        framebuffer->BaseAddress) = 0xffffffff;
    }
  }
  return;
}
