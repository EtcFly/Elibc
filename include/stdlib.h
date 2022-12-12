#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#ifdef __cplusplus
extern "C"
{
#endif
    long long int llabs(long long int j);
    long int labs(long int j);
    int abs(int j);

    long atol(const char *nptr);
    int atoi(const char *nptr);
    long strtol(const char *__restrict str, char **__restrict endptr,
                int base);

#ifdef __cplusplus
}
#endif

#endif
