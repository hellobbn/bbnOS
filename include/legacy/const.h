//===- const.h - Consts --------------------------------------------------===//
//
// This file defines basic constants
//
//===---------------------------------------------------------------------===//

#ifndef CONST_H
#define CONST_H

#define PUBLIC
#define PRIVATE static

#ifndef EXTERN
#define EXTERN extern
#endif

#define GDT_SIZE 128
#define IDT_SIZE 256

// privilige
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

#endif