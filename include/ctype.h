#ifndef _LIBC_CTYPE_H
#define _LIBC_CTYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

    typedef __libc_uint16_t __ctype_mask_t;

#define __UCLIBC_CTYPE_IN_TO_DOMAIN(c) (((unsigned int)(c)) < 256)

#ifndef _ISbit
/* These are all the characteristics of characters.
   If there get to be more than 16 distinct characteristics,
   __ctype_mask_t will need to be adjusted. */

/* libstdc++ from gcc toolchain needs this macro. */
#define _ISbit(bit) (1 << (bit))

#endif

    enum
    {
        _ISupper = _ISbit(0),  /* UPPERCASE.  */
        _ISlower = _ISbit(1),  /* lowercase.  */
        _ISalpha = _ISbit(2),  /* Alphabetic.  */
        _ISdigit = _ISbit(3),  /* Numeric.  */
        _ISxdigit = _ISbit(4), /* Hexadecimal numeric.  */
        _ISspace = _ISbit(5),  /* Whitespace.  */
        _ISprint = _ISbit(6),  /* Printing.  */
        _ISgraph = _ISbit(7),  /* Graphical.  */
        _ISblank = _ISbit(8),  /* Blank (usually SPC and TAB).  */
        _IScntrl = _ISbit(9),  /* Control character.  */
        _ISpunct = _ISbit(10), /* Punctuation.  */
        _ISalnum = _ISbit(11)  /* Alphanumeric.  */
    };

    extern const __ctype_mask_t *__ctype_b;
#define __isctype(c, type) ((__ctype_b)[(int)(c)] & (__ctype_mask_t)type)

#define isalnum(c) __isctype((c), _ISalnum)
#define isalpha(c) __isctype((c), _ISalpha)
#define iscntrl(c) __isctype((c), _IScntrl)
#define isdigit(c) __isctype((c), _ISdigit)
#define islower(c) __isctype((c), _ISlower)
#define isgraph(c) __isctype((c), _ISgraph)
#define isprint(c) __isctype((c), _ISprint)
#define ispunct(c) __isctype((c), _ISpunct)
#define isspace(c) __isctype((c), _ISspace)
#define isupper(c) __isctype((c), _ISupper)
#define isxdigit(c) __isctype((c), _ISxdigit)
#define isblank(c) __isctype((c), _ISblank)

    int tolower(int c);
    int toupper(int c);

#ifdef __cplusplus
}
#endif

#endif
