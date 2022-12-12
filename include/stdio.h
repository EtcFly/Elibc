#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"
#include "stdarg.h"

    int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
    int printf(const char *__restrict format, ...);

#ifdef __cplusplus
}
#endif

#endif
