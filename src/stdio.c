#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"
#include "local_console.h"

#define ZEROPAD (1 << 0) /* pad with zero */
#define SIGN (1 << 1)    /* unsigned/signed long */
#define PLUS (1 << 2)    /* show plus */
#define SPACE (1 << 3)   /* space if plus */
#define LEFT (1 << 4)    /* left justified */
#define SPECIAL (1 << 5) /* 0x */
#define LARGE (1 << 6)   /* use 'ABCDEF' instead of 'abcdef' */

int skip_atoi(const char **s)
{
    int i = 0;
    while (isdigit(**s))
        i = i * 10 + *((*s)++) - '0';

    return i;
}

int divide(long *n, int base)
{
    int res;

    /* optimized for processor which does not support divide instructions. */
    if (base == 10)
    {
        res = (int)(((unsigned long)*n) % 10U);
        *n = (long)(((unsigned long)*n) / 10U);
    }
    else
    {
        res = (int)(((unsigned long)*n) % 16U);
        *n = (long)(((unsigned long)*n) / 16U);
    }

    return res;
}

static char *print_number(char *buf,
                          char *end,
                          long num,
                          int base,
                          int s,
                          int precision,
                          int type)
{
    char c, sign;
    char tmp[16];
    int precision_bak = precision;
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    int i, size;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
        }
        else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }

    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
    {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }

    if (i > precision)
        precision = i;
    size -= precision;

    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
            size--;

        while (size-- > 0)
        {
            if (buf < end)
                *buf = ' ';
            ++buf;
        }
    }

    if (sign)
    {
        if (buf < end)
        {
            *buf = sign;
        }
        --size;
        ++buf;
    }

    if (type & SPECIAL)
    {
        if (base == 8)
        {
            if (buf < end)
                *buf = '0';
            ++buf;
        }
        else if (base == 16)
        {
            if (buf < end)
                *buf = '0';
            ++buf;
            if (buf < end)
            {
                *buf = type & LARGE ? 'X' : 'x';
            }
            ++buf;
        }
    }

    /* no align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf < end)
                *buf = c;
            ++buf;
        }
    }

    while (i < precision--)
    {
        if (buf < end)
            *buf = '0';
        ++buf;
    }

    /* put number in the temporary buffer */
    while (i-- > 0 && (precision_bak != 0))
    {
        if (buf < end)
            *buf = tmp[i];
        ++buf;
    }

    while (size-- > 0)
    {
        if (buf < end)
            *buf = ' ';
        ++buf;
    }

    return buf;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    __libc_uint32_t num;

    int i, len;
    char *str, *end, c;
    const char *s;

    __libc_uint8_t base;        /* the base of number */
    __libc_uint8_t flags;       /* flags to print number */
    __libc_uint8_t qualifier;   /* 'h', 'l', or 'L' for integer fields */
    __libc_int32_t field_width; /* width of output field */
    int precision;              /* min. # of digits for integers and max for a string */

    str = buf;
    end = buf + size;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end = ((char *)-1);
        size = end - buf;
    }

    for (; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str < end)
                *str = *fmt;
            ++str;
            continue;
        }

        /* process flags */
        flags = 0;

        while (1)
        {
            /* skips the first '%' also */
            ++fmt;
            if (*fmt == '-')
                flags |= LEFT;
            else if (*fmt == '+')
                flags |= PLUS;
            else if (*fmt == ' ')
                flags |= SPACE;
            else if (*fmt == '#')
                flags |= SPECIAL;
            else if (*fmt == '0')
                flags |= ZEROPAD;
            else
                break;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++fmt;
            if (isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = 0;
        if (*fmt == 'h' || *fmt == 'l')
        {
            qualifier = *fmt;
            ++fmt;
        }

        /* the default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str < end)
                        *str = ' ';
                    ++str;
                }
            }

            /* get character */
            c = (__libc_uint8_t)va_arg(args, int);
            if (str < end)
                *str = c;
            ++str;

            /* put width */
            while (--field_width > 0)
            {
                if (str < end)
                    *str = ' ';
                ++str;
            }
            continue;

        case 's':
            s = va_arg(args, char *);
            if (!s)
                s = "(NULL)";

            for (len = 0; (len != field_width) && (s[len] != '\0'); len++)
                ;

            if (precision > 0 && len > precision)
                len = precision;

            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str < end)
                        *str = ' ';
                    ++str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str < end)
                    *str = *s;
                ++str;
                ++s;
            }

            while (len < field_width--)
            {
                if (str < end)
                    *str = ' ';
                ++str;
            }
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
                flags |= ZEROPAD;
            }

            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, precision, flags);
            continue;

        case '%':
            if (str < end)
                *str = '%';
            ++str;
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (str < end)
                *str = '%';
            ++str;

            if (*fmt)
            {
                if (str < end)
                    *str = *fmt;
                ++str;
            }
            else
            {
                --fmt;
            }
            continue;
        }

        if (qualifier == 'l')
        {
            num = va_arg(args, __libc_uint32_t);
            if (flags & SIGN)
                num = (__libc_int32_t)num;
        }
        else if (qualifier == 'h')
        {
            num = (__libc_uint16_t)va_arg(args, __libc_int32_t);
            if (flags & SIGN)
                num = (__libc_int16_t)num;
        }
        else
        {
            num = va_arg(args, __libc_uint32_t);
            if (flags & SIGN)
                num = (__libc_int32_t)num;
        }

        str = print_number(str, end, num, base, field_width, precision, flags);
    }

    if (size > 0)
    {
        if (str < end)
            *str = '\0';
        else
        {
            end[-1] = '\0';
        }
    }

    /* the trailing null byte doesn't count towards the total
     * ++str;
     */
    return str - buf;
}

int printf(const char *__restrict format, ...)
{
    static char log_buf[512];
    va_list args;
    int length;

    va_start(args, format);
    length = vsnprintf(log_buf, sizeof(log_buf) - 1, format, args);

    // print ....
    lc_console_callback_t __lc_console_cb = get_active_console();
    if (NULL != __lc_console_cb)
    {
        __lc_console_cb(log_buf);
    }

    va_end(args);

    return length;
}
