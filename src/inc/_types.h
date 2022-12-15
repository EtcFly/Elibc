#ifndef __INTERNAL_LIBC_TYPES_H
#define __INTERNAL_LIBC_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* Convenience types.  */
    typedef unsigned char __libc_u_char;
    typedef unsigned short int __libc_u_short;
    typedef unsigned int __libc_u_int;
    typedef unsigned long int __libc_u_long;

    /* Fixed-size types, underlying types depend on word size and compiler.  */
    typedef signed char __libc_int8_t;
    typedef unsigned char __libc_uint8_t;
    typedef signed short int __libc_int16_t;
    typedef unsigned short int __libc_uint16_t;
    typedef signed int __libc_int32_t;
    typedef unsigned int __libc_uint32_t;

    typedef signed long int __libc_int64_t;
    typedef unsigned long int __libc_uint64_t;

    typedef long int __libc_intmax_t;
    typedef unsigned long int __libc_uintmax_t;
    typedef __libc_uint32_t size_t;

#define NULL ((void *)0)

#ifndef __S16_TYPE
#define __S16_TYPE short int
#endif

#ifndef __U16_TYPE
#define __U16_TYPE unsigned short int
#endif

#ifndef __S32_TYPE
#define __S32_TYPE int
#endif

#ifndef __U32_TYPE
#define __U32_TYPE unsigned int
#endif

#ifndef __SLONGWORD_TYPE
#define __SLONGWORD_TYPE long int
#endif

#ifndef __ULONGWORD_TYPE
#define __ULONGWORD_TYPE unsigned long int
#endif

#ifdef __cplusplus
}
#endif

#endif
