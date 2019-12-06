# bbnOS

## A. Overview

This is an Operating System by BBN following `OrangeS Operating System`'s guide.

## B. Building

### B.1 Build

First you should have necessary tools installed, then run the following command:

```bash
$ make
```
and the output image will be in the build/ dir

### B.2 run

The OS needs QEMU to run:

```bash
make qemu   # one possible command
# or you can run manually
make
qemu-system-i386 -fda build/qemu
```

## C. Notes

The filesystem is a bit out-dated, so maybe I can use FAT32 for the filesystem?
TODO MARKED!