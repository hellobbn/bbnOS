///===- mouse.c - Mouse related implementations --------------------------===///
/// Implements basic PS/2 mouse functions
///===--------------------------------------------------------------------===///

#include "types.h"
#include "io.h"
#include "mouse.h"
#include "fb.h"

static uint8_t mouseCycle = 0;
static uint8_t mousePacket[4];
static bool mousePacketReady = false;

// wait_write
static void PS2MouseWait() {
  uint64_t timeout = 100000;
  while (timeout--) {
    if ((inb(0x64) & 0b10) == 0) {
      return;
    }
  }
}

// wait_read
static void PS2MouseWaitInput() {
  uint64_t timeout = 100000;
  while (timeout--) {
    if (inb(0x64) & 0b1) {
      return;
    }
  }
}

void MouseWrite(uint8_t value) {
  PS2MouseWait();
  outb(0x64, 0xD4);
  PS2MouseWait();
  outb(0x60, value);
}

uint8_t MouseRead() {
  PS2MouseWaitInput();
  return inb(0x60);
}

void PS2MouseInit() {
  // disable keyboard
  PS2MouseWait();
  outb(0x64, 0xad);
  PS2MouseWaitInput();
  inb(0x60);
  PS2MouseWait();

  // Enable Auxiliary Device command to 0x64
  outb(0x64, 0xA8);

  PS2MouseWait();

  outb(0x64, 0x20);

  PS2MouseWaitInput();

  uint8_t status = inb(0x60);
  status |= 0b10;
  PS2MouseWait();

  outb(0x64, 0x60);
  PS2MouseWait();
  outb(0x60, status);

  MouseWrite(0xF6);
  MouseRead();

  MouseWrite(0xF4);
  MouseRead();

  // enable keyboard
  PS2MouseWait();
  outb(0x64, 0xae);
  PS2MouseWaitInput();
  inb(0x60);
}

void handlePS2Mouse(uint8_t data) {
  switch (mouseCycle) {
  case 0:
    if (mousePacketReady) {
      break;
    }
    if ((data & (0b00001000)) == 0) {
      break;
    }
    mousePacket[0] = data;
    mouseCycle++;
    break;
  case 1:
    if (mousePacketReady) {
      break;
    }
    mousePacket[1] = data;
    mouseCycle++;
    break;
  case 2:
    if (mousePacketReady) {
      break;
    }
    mousePacket[2] = data;
    mousePacketReady = true;
    mouseCycle = 0;
    break;
  }
}

void ProcessMousePacket() {
  static uint64_t mousePosX = 0;
  static uint64_t mousePosY = 0;
  if (!mousePacketReady)
    return;

  int xNeg, yNeg;
  bool xOver, yOver;
  if (mousePacket[0] & PS2XSign) {
    xNeg = -1;
  } else {
    xNeg = 1;
  }

  if (mousePacket[0] & PS2YSign) {
    yNeg = -1;
  } else {
    yNeg = 1;
  }

  if (mousePacket[0] & PS2YOverflow) {
    yOver = true;
  } else {
    yOver = false;
  }

  if (mousePacket[0] & PS2XOverflow) {
    xOver = true;
  } else {
    xOver = false;
  }


  if (xNeg == -1) {
    mousePacket[1] = 256 - mousePacket[1];
  }

  if (yNeg == -1) {
    mousePacket[2] = 256 - mousePacket[2];
  }

  mousePosX += xNeg * mousePacket[1];
  if (xOver) {
    mousePosX += xNeg * 255;
  }

  mousePosY -= yNeg * mousePacket[2];
  if (yOver) {
    mousePosY -= yNeg * 255;
  }

  if (mousePosX < 0) {
    mousePosX = 0;
  } else if (mousePosX > fb_info.Width - 1) {
    mousePosX = fb_info.Width - 1;
  }

  if (mousePosY < 0) {
    mousePosY = 0;
  } else if (mousePosY > fb_info.Height - 1) {
    mousePosY = fb_info.Height - 1;
  }
  
  mousePacketReady = false;

  _fbPutChar(0xffffffff, 'a', mousePosX, mousePosY);
}