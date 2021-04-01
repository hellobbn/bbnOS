///===- valist.h - Simple va_list implementation ------------------------===///
///
/// Simple valist implementation.
///
///===-------------------------------------------------------------------===///
//
// FIXME: We are using the built-in valist implementation here.
//        We should create our own version of va.
//
//===--------------------------------------------------------------------===///
#ifndef VALIST_H
#define VALIST_H

// typedef void* va_list;
// #define va_start(ap, v) ((void) (ap = (va_list) &v + sizeof(v)))
// #define va_end(ap) ((void) (ap = 0))
// #define va_arg(ap, type) (*((type *)(ap)) ++)

#define va_list     __builtin_va_list
#define va_start(v, f) __builtin_va_start(v, f)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v, a)   (__builtin_va_arg(v, a))

#endif // VALIST_H
