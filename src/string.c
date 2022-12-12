#include "string.h"

void *memset(void *s, int c, size_t n)
{
    register char *ps = s;
    size_t __n = n;

    while (ps && __n-- > 0)
    {
        *ps++ = (char)c;
    }
    return 0;
}

void *memcpy(void *__restrict s1, const void *__restrict s2, size_t n)
{
    register char *r1 = s1;
    register const char *r2 = s2;

    while (n)
    {
        *r1++ = *r2++;
        --n;
    }

    return s1;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    register const unsigned char *r1 = (const unsigned char *)s1;
    register const unsigned char *r2 = (const unsigned char *)s2;

    int r = 0;

    while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0))
        ;

    return r;
}

char *stpcpy(register char *s1, const char *s2)
{
    while ((*s1++ = *s2++) != 0)
        ;

    return s1 - 1;
}

char *strcpy(char *__restrict s1, const char *__restrict s2)
{
    register char *s = s1;

    while ((*s++ = *s2++) != 0)
        ;

    return s1;
}

char *strncpy(char *__restrict s1, register const char *__restrict s2,
              size_t n)
{
    register char *s = s1;

    while (n)
    {
        if ((*s = *s2) != 0)
            s2++; /* Need to fill tail with 0s. */
        ++s;
        --n;
    }

    return s1;
}

size_t strlen(const char *s)
{
    register const char *p;

    for (p = s; *p; p++)
        ;

    return p - s;
}

char *strcat(char *__restrict s1, register const char *__restrict s2)
{
    register char *s = s1;

    while (*s++)
        ;
    --s;
    while ((*s++ = *s2++) != 0)
        ;

    return s1;
}

int strcmp(register const char *s1, register const char *s2)
{
    int r;

    while (((r = ((int)(*((unsigned char *)s1))) - *((unsigned char *)s2++)) == 0) && *s1++)
        ;

    return r;
}
