#ifndef __INTERNAL_LIBC_STDARG_H
#define __INTERNAL_LIBC_STDARG_H

#ifdef __cplusplus
extern "C"
{
#endif

#define va_list __builtin_va_list
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || __cplusplus + 0 >= 201103L
#define va_copy(d, s) __builtin_va_copy(d, s)
#endif
#define __va_copy(d, s) __builtin_va_copy(d, s)

    // #include "types.h"

#ifdef __cplusplus
}
#endif

#endif
