///===- io.c - I/O Operations --------------------------------------------===///
/// Functions for communicating with Memory Mapped I/O
///===--------------------------------------------------------------------===///

#include "io.h"
#include "types.h"

void outb(uint16_t port, uint8_t data) {
  asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void io_wait() { asm volatile("nop"); }
