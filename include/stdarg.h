#ifndef _LIBC_STDARG_H
#define _LIBC_STDARG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../src/inc/_stdarg.h"

#ifndef va_list
#define va_list
#endif

#ifndef va_start
#define va_start(v, l)
#endif

#ifndef va_end
#define va_end(v)
#endif

#ifndef va_arg
#define va_arg(v, l)
#endif

#ifndef va_copy
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || __cplusplus + 0 >= 201103L
#define va_copy(d, s) __builtin_va_copy(d, s)
#endif
#endif

#ifndef __va_copy
#define __va_copy(d, s)
#endif

#ifdef __cplusplus
}
#endif

#endif
