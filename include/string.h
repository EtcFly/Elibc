#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

#ifndef __restrict
#define __restrict
#endif

    void *memset(void *s, int c, size_t n);
    void *memcpy(void *__restrict s1, const void *__restrict s2, size_t n);
    int memcmp(const void *s1, const void *s2, size_t n);

    char *stpcpy(register char *s1, const char *s2);
    char *strcpy(char *__restrict s1, const char *__restrict s2);
    char *strncpy(char *__restrict s1, register const char *__restrict s2,
                  size_t n);
    size_t strlen(const char *s);
    char *strcat(char *__restrict s1, register const char *__restrict s2);
    int strcmp(register const char *s1, register const char *s2);

#ifdef __cplusplus
}
#endif

#endif
