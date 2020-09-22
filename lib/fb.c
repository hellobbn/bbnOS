//===- fb.c - framebuffer ------------------------------------------------===//
//
// Implementation of framebuffer functions.
//
// TODO: screen scroll
//
//===---------------------------------------------------------------------===//

#include "fb.h"
#include "const.h"
#include "io.h"

/// fb points to the frame buffer.
PRIVATE char *fb = (char *)0x000B8000;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
  fb[i] = c;
  fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

/// Move the cursor to the new position \p pos
/// Use `outb` function to move the cursor of the framebuffer to the given
/// position The cursor position is a 16-bit integer: 0->row 0, column 0;
/// 80->row 1, column 0.
/// The position is 16-bit so need to be sent twice(outb is 8-bit)
static void fb_move_cursor(unsigned short pos) {
  outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
  outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
  outb(FB_DATA_PORT, (pos & 0x00FF));
}

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

void fb_clear() {
  int all = 80 * 25 * 2;
  for (int i = 0; i < all; i += 2) {
    fb_write_cell(i, 0x20, FB_CLOR_WHITE, FB_CLOR_BLACK);
  }

  fb_move_cursor(1);
}

void fb_print_color(char *buf, int flag, unsigned int fg, unsigned int bg) {
  static int x_pos = 0;
  static int y_pos = 0;

  // if secondly prints, clear first
  static int clear = 0;
  unsigned int pos;
  int cnt = 0;
  char c;

  if (x_pos > 100 || y_pos > 100) {
    x_pos = 0;
    y_pos = 0;
    print("error");
    return;
  }

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
      if (clear == 1) {
        for (int i = 0; i < 80; ++i) {
          pos = (y_pos * 80 + i) * 2;
          fb_write_cell(pos, ' ', fg, bg);
          fb_move_cursor(y_pos * 80 + i + 1);
        }
      }
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
      clear = 1;
    }

    cnt++;
  }
}

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

void fb_print_dec(int num) {
  if (num >= 10000000) {
    num = num % 10000000;
    // return; // the number is too large, the stack is small.
  }
  char stack[10];
  int index = 0;

  char buff[2];
  if (num < 0) {
    print("-");
    num = -num;
  }
  if (num == 0) {
    buff[0] = '0';
    buff[1] = 0;
    print(buff);
    return;
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
