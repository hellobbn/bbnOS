/* I/O header file */
#ifndef IO_H
#define IO_H

/** outb:
 *  Function from nasm file: loader.s->outb
 *  Sends the given data to the given I/O port: 8 bits at a time
 *  implemented in lib/io.asm
 * 
 *  @param port     The I/O port to send data
 *  @param data     The data to be sent
 */
void outb(unsigned int port, unsigned char data);

/** inb:
 *  Read a byte from the given I/O port
 *  implememted in lib/io.asm
 * 
 *  @param port The address of the I/O port
 *  @return     The read byte
 */
unsigned int inb(unsigned short port);

#endif
