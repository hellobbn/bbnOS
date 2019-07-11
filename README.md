# bbnOS

## A. Overview

This is an Operating System by BBN following `OrangeS Operating System`'s guide.

## B. Building

### B.1 Initialize

In this branch, the bootsect may get out of 512-byte, so we are converting the OS to a COM file, using FreeDOS to run it. Put the FreeDOS floppy in `deps/`, and use command `dd if=/dev/zero of=deps/floppy_boot.img bs=512 count=2880` to make boot floppy. In DOS, format floppy B. 

### B.2 Build

Run the following command:

```bash
./run.sh
``` 

and in DOS window, run:

```dos
b:\boot.com
```