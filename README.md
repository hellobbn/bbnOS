# bbnOS

## A. Overview

This is an Operating System by BBN following `OrangeS Operating System`'s guide.

## B. Building

### B.1 Initialize

In this branch, the bootsect may get out of 512-byte, so we are converting the OS to a COM file, using FreeDOS to run it. Put the FreeDOS floppy in `deps/`, and use command 

```bash
dd if=/dev/zero of=deps/floppy_boot.img bs=512 count=2880
``` 

to make boot floppy. In DOS, format floppy B. 

### B.2 Builds

Run the following command:

```bash
make 
``` 

### B.3 Run

As is mentioned, the system is loaded using FreeDOS,

```bash
make freedos
```s

and in DOS window, run:

```dos
b:\boot.com
```