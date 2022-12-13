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
    int local_printf(const char *__restrict format, ...);

    void mem_heap_init(void *begin_addr, void *end_addr);
    void *malloc(size_t size);
    void *realloc(void *rmem, size_t newsize);
    void *calloc(size_t count, size_t size);
    void free(void *rmem);
#ifdef __cplusplus
}
#endif

#endif
