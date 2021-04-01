///===- fb.h - Framebuffer operations -----------------------------------===///
/// Framebuffer operations
///===-------------------------------------------------------------------===///
///
/// This file implements the framebuffer for UEFI kernel. The framebuffer
/// address location is passed from the kmain function and copied to this
/// file's static field, so the kernel should first call the `setup_fb`
/// function to let the tools understand the framebuffer position.
/// The actual framebuffer takes place in this file, and when it comes to
/// writing the framebuffer, other functions who want to write to framebuffer
/// should always call functions declared here.
///
///===-------------------------------------------------------------------===///
//
// TODO: Function naming is not consistent
//
//===--------------------------------------------------------------------===///

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "types.h"

#define FB_OP_SUCCESS 0
#define FB_OP_FAIL 1

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

/// This function initializes the framebuffer with the location and font.
///
/// \param fb The structure describing the framebuffer
/// \param ft The font to be used in the printing function
/// \return Whether the initialization is successful, FB_OP_SUCCESS if success
int initFb(Framebuffer *fb, PSF1_FONT *ft);

/// This **UNSAFE** function sets the framebuffer to the parameter, used for
/// purposes only.
///
/// \param fb The target framebuffer we are to set
/// \return FB_OP_SUCCESS on success, FB_OP_FAIL on failure
int setFB(Framebuffer *fb);

/// This function changes the font of the framebuffer. Note that this will NOT
/// changes the fonts which are already printed in the framebuffer. Only newly
/// printed fonts are affected.
///
/// \param ft The font to set to
/// \return FB_OP_SUCCESS on success, FB_OP_FAIL on failure
int setFont(PSF1_FONT *ft);

/// The movable putchar function. Print a character to screen and move to next
/// position
///
/// \param c The character to print
void fb_putchar(char c);

/// Print a string in formatted style. Use it as normal printf function.
int printf(const char *format, ...);

/// Writes a character to the framebuffer, we moves the cursor alongside.
/// This actually calls the functions in fb.c.
/// FIXME: This actually takes much context switch time, we need to clean them
///        up eventually
///
#define putchar(c) (fb_putchar(c))

#endif // FRAMEBUFFER_H
