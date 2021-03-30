/* I/O header file */
#ifndef IO_H
#define IO_H

///  Function from nasm file: loader.s->outb, Sends the given \p data to the
/// given I/O port \p port : 8 bits at a time.
/// Implemented in lib/io.asm
void outb(unsigned int port, unsigned char data);

/// Read a byte from the given I/O port \p port.
/// Implememted in lib/io.asm
/// \return The read data
unsigned int inb(unsigned short port);

#endif
