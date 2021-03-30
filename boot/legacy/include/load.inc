; include file for loader
BaseOfLoader        equ     0x9000
OffsetOfLoader      equ     0x100

BaseOfLoaderPhyAddr equ     BaseOfLoader * 10h

BaseOfKernelFile    equ     0x8000
OffsetOfKernelFile  equ     0x0

BaseOfKernelFilePhyAddr equ     BaseOfKernelFile * 10h
KernelEntryPointPhyAddr equ     030400h