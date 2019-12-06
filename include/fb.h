/* Framebuffer header file */

#ifndef FB_H
#define FB_H

/* Functions defined here */
int fb_writer(char* buf, unsigned int column, unsigned int row);
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_clear();
void fb_print(char* buf);
void fb_print_hex(unsigned int num);

/* Framebuffer colors starts here */
#define     FB_CLOR_BLACK           0
#define     FB_CLOR_BLUE            1
#define     FB_CLOR_GREEN           2
#define     FB_CLOR_CYAN            3
#define     FB_CLOR_RED             4
#define     FB_CLOR_MAGENTA         5
#define     FB_CLOR_BROWN           6
#define     FB_CLOR_LGREY           7
#define     FB_CLOR_DGREY           8
#define     FB_CLOR_LBLUE           9
#define     FB_CLOR_LGREEN          10
#define     FB_CLOR_LCYAN           11
#define     FB_CLOR_LRED            12
#define     FB_CLOR_LMAGENTA        13
#define     FB_CLOR_LBROWN          14
#define     FB_CLOR_WHITE           15

/* Framebuffer I/O ports */
#define     FB_COMMAND_PORT         0x3D4   // port thar describes the data
#define     FB_DATA_PORT            0x3D5   // port for the data itself

/* Framebuffer I/O port commands */
#define     FB_HIGH_BYTE_COMMAND    14
#define     FB_LOW_BYTE_COMMAND     15

#endif
