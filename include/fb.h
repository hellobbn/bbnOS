//===- fb.h - Frame Buffer functions -------------------------------------===//
//
// This header declares basic frame buffer interfaces.
//
//===---------------------------------------------------------------------===//
#ifndef FB_H
#define FB_H

/// Frame buffer functions
///{

/// Writes a string \p buf to the framebuffer.
int fb_writer(char *buf, unsigned int column, unsigned int row);

/// Writes a character \p c to position \p i with the given foreground \p fg
/// and background \p bg in the framebuffer.
/// The starting address of the memory-mapped I/O for the framebuffer is
/// 0x000B8000 and the memory is diveded into 16-bit cells as follows:
/// --------------------------------------------------------------------
/// Bit:      | 15 14 13 12 11 10 9 8 | 7 6 5 4 | 3 2 1 0 |
/// Content:  | ASCII                 | FG      | BG      |
/// --------------------------------------------------------------------
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);

/// Clear the framebuffer.
/// This actually fills the framebuffer with spaces and black background.
void fb_clear();

/// Another print function, with color support.
/// Prints the \p buf with given foreground \p fg and background \p bg
/// It automatically moves cursor and the position. Note that if you write
/// something with `fb_writer()`, then the use of this function may override
/// the string printed before.
/// Also note that if the position is out of boundary, it will start over to
/// (0, 0)
/// The \p flag is normally set to 0 and it will cause the function to do a
/// force reset of the cursor if set to 1.
void fb_print_color(char *buf, int flag, unsigned int fg, unsigned int bg);

/// Prints a hex number. This will call the `fb_print_color()` function so that
/// the output will not be messed up.
/// For now we don't expose the color here.
void fb_print_hex(unsigned int num);

/// Prints a dec number. It will call the `fb_print_color()` function so that
/// the output will not be messed up.
/// For now we don't expose the color here.
/// Very inefficient
void fb_print_dec(int num);
///}

/// Simple interface for print
///{
#define print(msg) fb_print((msg), 0);
#define fb_print(msg, flag)                                                    \
  fb_print_color(msg, flag, FB_CLOR_WHITE, FB_CLOR_BLACK);
///}

/// Framebuffer colors
///{
#define FB_CLOR_BLACK 0
#define FB_CLOR_BLUE 1
#define FB_CLOR_GREEN 2
#define FB_CLOR_CYAN 3
#define FB_CLOR_RED 4
#define FB_CLOR_MAGENTA 5
#define FB_CLOR_BROWN 6
#define FB_CLOR_LGREY 7
#define FB_CLOR_DGREY 8
#define FB_CLOR_LBLUE 9
#define FB_CLOR_LGREEN 10
#define FB_CLOR_LCYAN 11
#define FB_CLOR_LRED 12
#define FB_CLOR_LMAGENTA 13
#define FB_CLOR_LBROWN 14
#define FB_CLOR_WHITE 15
///}

/// Framebuffer IO Ports
///{

/// Port that decribes the data.
#define FB_COMMAND_PORT 0x3D4

/// Port fot the data itself.
#define FB_DATA_PORT 0x3D5
///}

/// Framebuffer commands
///{
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15
///}

#endif
