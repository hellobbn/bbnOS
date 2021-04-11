///===- Io.h - I/O Operations --------------------------------------------===///
/// Functions for communicating with Memory Mapped I/O
///===--------------------------------------------------------------------===///
///
/// Functions (inb, outb) here can be used to send bytes to memory mapped I/O,
/// allowing the kernel to communicate with various devices using commands, and
/// get data from memory
///
///===--------------------------------------------------------------------===///
#ifndef IO_H
#define IO_H

#include "types.h"

/// Sends the given data to the given I/O port port : 8 bits at a time.
///
/// \param port The I/O Port
/// \param data The data to send to the port
void outb(uint16_t port, uint8_t data);

/// Read a byte from the given I/O port \p port.
///
/// \param port The I/O port
/// \return The read data
uint8_t inb(uint16_t port);

/// Block function, wait for some I/O operations to complete
void io_wait();

#endif