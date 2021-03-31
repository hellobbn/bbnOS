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

/// Put a char with specified color to the specific position in the framebuffer
///
/// \param color The 32-bit true color (maybe)
/// \param c The char to be put to screen
/// \param pos_x The x position of the font (column)
/// \param pos_y The y position of the font (row)
void fbPutChar(uint32_t color, char c, unsigned int pos_x, unsigned int pos_y);

#endif // FRAMEBUFFER_H
