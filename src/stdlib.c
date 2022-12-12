#include "stdlib.h"
#include "limits.h"
#include "ctype.h"
#include "assert.h"

int abs(int j)
{
    return (j >= 0) ? j : -j;
}

long int labs(long int j)
{
    return (j >= 0) ? j : -j;
}

long long int llabs(long long int j)
{
    return (j >= 0) ? j : -j;
}

unsigned long _stdlib_strto_l(register const char *__restrict str,
                              char **__restrict endptr, int base,
                              int sflag)
{
    unsigned long number, cutoff;
    const char *fail_char;
#define SET_FAIL(X) fail_char = (X)

    unsigned char negative, digit, cutoff_digit;

    assert(((unsigned int)sflag) <= 1);

    SET_FAIL(str);

    while (isspace(*str))
    { /* Skip leading whitespace. */
        ++str;
    }

    /* Handle optional sign. */
    negative = 0;
    switch (*str)
    {
    case '-':
        negative = 1; /* Fall through to increment str. */
    case '+':
        ++str;
    }

    if (!(base & ~0x10))
    {               /* Either dynamic (base = 0) or base 16. */
        base += 10; /* Default is 10 (26). */
        if (*str == '0')
        {
            SET_FAIL(++str);
            base -= 2; /* Now base is 8 or 16 (24). */
            if ((0x20 | (*str)) == 'x')
            { /* WARNING: assumes ascii. */
                ++str;
                base += base; /* Base is 16 (16 or 48). */
            }
        }

        if (base > 16)
        { /* Adjust in case base wasn't dynamic. */
            base = 16;
        }
    }

    number = 0;

    if (((unsigned)(base - 2)) < 35)
    { /* Legal base. */
        cutoff_digit = ULONG_MAX % base;
        cutoff = ULONG_MAX / base;
        do
        {
            digit = ((unsigned char)(*str - '0') <= 9)
                        ? /* 0..9 */ (*str - '0')
                        : /* else */ (((unsigned char)(0x20 | *str) >= 'a') /* WARNING: assumes ascii. */
                                          ? /* >= A/a */ ((unsigned char)(0x20 | *str) - ('a' - 10))
                                          : /* else   */ 40 /* bad value */);

            if (digit >= base)
            {
                break;
            }

            SET_FAIL(++str);

            if ((number > cutoff) || ((number == cutoff) && (digit > cutoff_digit)))
            {
                number = ULONG_MAX;
                negative &= sflag;
            }
            else
            {
                number = number * base + digit;
            }
        } while (1);
    }

    if (endptr)
    {
        *endptr = (char *)fail_char;
    }

    {
        unsigned long tmp = (negative
                                 ? ((unsigned long)(-(1 + LONG_MIN))) + 1
                                 : LONG_MAX);
        if (sflag && (number > tmp))
        {
            number = tmp;
        }
    }

    return negative ? (unsigned long)(-((long)number)) : number;
}

long strtol(const char *__restrict str, char **__restrict endptr,
            int base)
{
    return _stdlib_strto_l(str, endptr, base, 1);
}

int atoi(const char *nptr)
{
    return (int)strtol(nptr, (char **)NULL, 10);
}

long atol(const char *nptr)
{
    return strtol(nptr, (char **)NULL, 10);
}