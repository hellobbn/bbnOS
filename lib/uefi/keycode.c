#include "keycode.h"
#include "fb.h"

// 0x00 - 0x53
static const char ASCIITable[] = {
    0, 0,    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    0, 0,    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
    0, 0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*',
    0, ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0, '7',  '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.'};

#define KEYCODE_ESCAPE 0x01
#define KEYCODE_BACKSPACE 0x0E
#define KEYCODE_TAB 0x0F
#define KEYCODE_ENTER 0x1C
#define KEYCODE_LCTRL 0x1D
#define KEYCODE_LSHIFT 0x2A
#define KEYCODE_RSHIFT 0x36
#define KEYCODE_LALT 0x38
#define KEYCODE_SPACE 0x39
#define KEYCODE_CAPSLOCK 0x3A
#define KEYCODE_F1 = 0x3B
#define KEYCODE_FN(X) (X > 10 ? (KEYCODE_F11 + X - 11) : (KEYCODE_F1 + X - 1))
#define KEYCODE_NUMLOCK 0x45
#define KEYCODE_SCROLLBACK 0x46
#define KEYCODE_FN11 0x57
#define KEYCODE_FN12 0x58

static const char upper_case_map[] = {
    '<', '_', '>', '?', ')', '!', '@', '#', '$', '%', '^', '&', '*', '(',
};

static char keycodeTranslate(uint8_t scancode, bool uppercase) {
  if (scancode > 0x53) {
    return 0;
  }

  char char_to_print = ASCIITable[scancode];

  if (char_to_print == 0) {
    return 0;
  }

  if (uppercase) {
    if (char_to_print >= 'a' && char_to_print <= 'z') {
      char_to_print += 'A' - 'a';
    } else if (char_to_print >= ',' && char_to_print <= '9') {
      char_to_print = upper_case_map[char_to_print - ','];
    } else {
      switch (char_to_print) {
      case ';':
        char_to_print = ':';
        break;
      case '\'':
        char_to_print = '"';
        break;
      case '[':
        char_to_print = '{';
        break;
      case ']':
        char_to_print = '}';
        break;
      case '\\':
        char_to_print = '|';
        break;
      case '`':
        char_to_print = '~';
        break;
      case '=':
        char_to_print = '+';
        break;
      default:
        char_to_print = char_to_print;
        break;
      }
    }
  }

  return char_to_print;
}

void keyboardHandle(uint8_t scancode) {
  static bool lshift = false;
  static bool rshift = false;
  static bool e0_prefix = false;
  bool pressed = false;

  if (scancode == 0xE0) {
    e0_prefix = true;
    return;
  }

  if (e0_prefix == 0) {
    if ((scancode & 0x80) == 0x80) {
      pressed = false;
      scancode = scancode - 0x80;
    } else {
      pressed = true;
    }

    switch (scancode) {
    case KEYCODE_LSHIFT:
      lshift = pressed;
      break;
    case KEYCODE_RSHIFT:
      rshift = false;
      break;
    case KEYCODE_ENTER:
      if (pressed) {
        printf("\n");
      }
      break;
    case KEYCODE_BACKSPACE:
      if (pressed) {
        fbClearChar();
      }
      break;
    case KEYCODE_SPACE:
      if (pressed) {
        printf(" ");
      }
      break;
    default:
      if (pressed == true) {
        char ascii = keycodeTranslate(scancode, lshift | rshift);

        if (ascii != 0) {
          printf("%c", ascii);
        }
      }
      break;
    }
  } else {
    printf("!! 0xE0 function not implemented\n");
    e0_prefix = false;
    return;
  }
}