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

    void rt_system_heap_init(void *begin_addr, void *end_addr);
    void *rt_malloc(size_t size);
    void *rt_realloc(void *rmem, size_t newsize);
    void *rt_calloc(size_t count, size_t size);
    void rt_free(void *rmem);
#ifdef __cplusplus
}
#endif

#endif
