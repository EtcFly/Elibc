#ifndef _LIBC_CTYPE_H
#define _LIBC_CTYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

#ifndef isalnum
#define isalnum(c)
#endif

#ifndef isalpha
#define isalpha(c)
#endif

#ifndef iscntrl
#define iscntrl(c)
#endif

#ifndef isdigit
#define isdigit(c)
#endif

#ifndef islower
#define islower(c)
#endif

#ifndef isgraph
#define isgraph(c)
#endif

#ifndef isprint
#define isprint(c)
#endif

#ifndef ispunct
#define ispunct(c)
#endif

#ifndef isspace
#define isspace(c)
#endif

#ifndef isupper
#define isupper(c)
#endif

#ifndef isxdigit
#define isxdigit(c)
#endif

#ifndef isblank
#define isblank(c)
#endif

    int tolower(int c);
    int toupper(int c);

#ifdef __cplusplus
}
#endif

#endif
