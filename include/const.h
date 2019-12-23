#ifndef CONST_H
#define CONST_H

#define PUBLIC
#define PRIVATE static
#define EXTERN  extern

#define GDT_SIZE 128
#define IDT_SIZE 256

// privilige
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

#endif