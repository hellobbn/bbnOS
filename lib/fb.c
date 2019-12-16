/* Framebuffer */
/* TODO: Scroll the screen */
/* TODO: Print Hex and other */

#include "fb.h"
#include "io.h"

char *fb = (char *)0x000B8000; // fb points to the start of the framebuffer

/** fb_write_cell:
 *  Writes a character with the given foreground and background to position i
 *  in the framebuffer
 *  The starting address of the memory-mapped I/O for the framebuffer is
 * 0x000B8000 and the memory is divided into 16-bit cells as follows:
 *  -----------------------------------------------------------------
 *  Bit:     | 15 14 13 12 11 10 9 8 | 7 6 5 4 | 3 2 1 0|
 *  Content: | ASCII                 | FG      | BG     |
 *  -----------------------------------------------------------------
 *  The colors for the FG and BG is defined in `fb.h` file
 *
 *  @param i    The location in the framebuffer
 *  @param c    The character
 *  @param fg   The foreground color
 *  @param bg   The background color
 *
 *  FIXME: The `fg` is actually the background
 *    NOTE: see http://www.jamesmolloy.co.uk/tutorial_html/3.-The%20Screen.html
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    fb[i] = c;
    fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

/** fb_move_cursor:
 *  Use `outb` function to move the cursor of the framebuffer to the given
 * position The cursor position is a 16-bit integer: 0->row 0, column 0; 80->row
 * 1, column 0 The position is 16-bit so need to be sent twice(outb is 8-bit)
 *
 *  @param pos  The new position of the cursor
 */
void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, (pos & 0x00FF));
}

/** fb_writer:
 *  Writes a string to the framebuffer
 *
 *  @param buf  The string to be written
 *  @param len  The length of the string
 *  @param column The column to be wirtten
 *  @param row  The row to be written
 */
int fb_writer(char *buf, unsigned int column, unsigned int row) {
    unsigned int cnt = 0;
    ;
    unsigned int record_x = column;
    unsigned int record_y = row;
    unsigned int pos;
    char c;

    while (buf[cnt]) {
        c = buf[cnt];

        // check c
        if (c == 0x08 && record_x) {
            record_x--;
        } else if (c == 0x09) {
            record_x = (record_x + 8) & ~(8 - 1);
        } else if (c == '\r') {
            record_x = 0;
        } else if (c == '\n') {
            record_x = 0;
            record_y++;
        } else if (c >= ' ') {
            pos = (record_y * 80 + record_x) * 2;
            fb_write_cell(pos, buf[cnt], FB_CLOR_WHITE, FB_CLOR_BLACK);
            fb_move_cursor(record_y * 80 + record_x + 1);
            record_x++;
        }

        if (record_x >= 80) {
            record_x = 0;
            record_y++;
        }

        cnt++;
    }
    return 0;
}

/** fb_clear:
 *  Clears the framebuffer, it actually fills the framebuffer with spaces with
 * black background
 */
void fb_clear() {
    int all = 80 * 25 * 2;
    for (int i = 0; i < all; i += 2) {
        fb_write_cell(i, 0x20, FB_CLOR_WHITE, FB_CLOR_BLACK);
    }

    fb_move_cursor(1);
}

/** fb_print_color:
 *  The actual print function, with color, it automatically moves cursor and
 * position. Note that if you write something using `fb_writer()`, then the use
 * of `fb_print` my override the string printed by `fb_writer()`
 *  @note the cursor moves, if the position is out of bondary, it will start
 * over
 *  TODO: safety check
 *
 *  @param buf  The input string, must be ended with `\0`
 *  @param flag set to 1 to reset cursor
 *  @param fg   forground color
 *  @param bg   background color
 */
void fb_print_color(char *buf, int flag, unsigned int fg, unsigned int bg) {
    static int x_pos = 0;
    static int y_pos = 0;
    unsigned int pos;
    int cnt = 0;
    char c;

    if (flag == 1) {
        x_pos = 0;
        y_pos = 0;
    }

    while (buf[cnt]) {
        c = buf[cnt];

        // check c
        if (c == 0x08 && x_pos) {
            x_pos--;
        } else if (c == 0x09) {
            x_pos = (x_pos + 8) & ~(8 - 1);
        } else if (c == '\r') {
            x_pos = 0;
        } else if (c == '\n') {
            x_pos = 0;
            y_pos++;
        } else if (c >= ' ') {
            pos = (y_pos * 80 + x_pos) * 2;
            fb_write_cell(pos, buf[cnt], fg, bg);
            fb_move_cursor(y_pos * 80 + x_pos + 1);
            x_pos++;
        }

        if (x_pos >= 80) {
            x_pos = 0;
            y_pos++;
        }

        if (y_pos >= 25) {
            x_pos = 0;
            y_pos = 0;
        }

        cnt++;
    }
}

/** fb_print_hex:
 * Prints a hex number, it will call `fb_print` so the cursor will not be messed
 * up
 *
 * @param num   The number in oct
 */
void fb_print_hex(unsigned int num) {
    fb_print("0x", 0);
    unsigned int tmp = num;
    int flag = 1;
    char rst;
    char a[2];
    for (int i = 28; i > 0; i -= 4) {
        rst = (tmp >> i) & 0xF;

        if (rst == 0 && flag) {
            continue;
        }

        if (rst < 10) {
            flag = 0;
            a[0] = rst + '0';
        } else if (rst >= 10 && rst <= 15) {
            flag = 0;
            a[0] = rst - 10 + 'A';
        }
        a[1] = 0;
        fb_print(a, 0);
    }

    rst = tmp & 0xF;
    if (rst >= 10) {
        a[0] = rst + 'a' - 0xA;
        a[1] = 0;
        fb_print(a, 0);
    } else {
        a[0] = rst + '0';
        a[1] = 0;
        fb_print(a, 0);
    }
}

/** fb_print_dec:
 *  print the number in dec, it will call `fb_print` so the cursor will not be
 * messed up the number can be signed The max length of the number is 7!
 *
 *  @param num   The number
 *  @note The function is very low in efficiency. Very, very low
 */
void fb_print_dec(int num) {
    if (num >= 10000000) {
        return; // the number is too large, the stack is small.
    }
    char stack[10];
    int index = 0;

    char buff[2];
    if (num < 0) {
        print("-");
        num = -num;
    }
    while (num != 0) {
        stack[index] = (char)(num % 10);
        index++;
        num = num / 10;
    }
    index -= 1;
    while (index >= 0) {
        // fb_print_hex(stack[index]);
        buff[0] = stack[index] + '0';
        buff[1] = 0;
        print(buff);
        index -= 1;
    }
}