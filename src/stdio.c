#include "stdio.h"
#include "string.h"
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

#if 1
struct rt_memory
{
    const char *algorithm;   /**< Memory management algorithm name */
    __libc_uint32_t address; /**< memory start address */
    size_t total;            /**< memory size */
    size_t used;             /**< size used */
    size_t max;              /**< maximum usage */
};

struct rt_small_mem_item
{
    __libc_uint32_t pool_ptr; /**< small memory object addr */
    size_t next;              /**< next free item */
    size_t prev;              /**< prev free item */
};

struct rt_small_mem
{
    struct rt_memory parent;  /**< inherit from rt_memory */
    __libc_uint8_t *heap_ptr; /**< pointer to the heap */
    struct rt_small_mem_item *heap_end;
    struct rt_small_mem_item *lfree;
    size_t mem_size_aligned; /**< aligned memory size */
};

#define HEAP_MAGIC 0x1ea0
#define MIN_SIZE 12
#define RT_ALIGN_SIZE 4
#define RT_ASSERT(...)

#define MEM_MASK 0xfffffffe
#define MEM_USED() ((((__libc_uint32_t)(small_mem)) & MEM_MASK) | 0x1)
#define MEM_FREED() ((((__libc_uint32_t)(small_mem)) & MEM_MASK) | 0x0)
#define MEM_ISUSED(_mem) \
    (((__libc_uint32_t)(((struct rt_small_mem_item *)(_mem))->pool_ptr)) & (~MEM_MASK))
#define MEM_POOL(_mem) \
    ((struct rt_small_mem *)(((__libc_uint32_t)(((struct rt_small_mem_item *)(_mem))->pool_ptr)) & (MEM_MASK)))
#define RT_ALIGN(size, align) (((size) + (align)-1) & ~((align)-1))
#define RT_ALIGN_DOWN(size, align) ((size) & ~((align)-1))
#define MEM_SIZE(_heap, _mem) \
    (((struct rt_small_mem_item *)(_mem))->next - ((__libc_uint32_t)(_mem) - (__libc_uint32_t)((_heap)->heap_ptr)) - RT_ALIGN(sizeof(struct rt_small_mem_item), RT_ALIGN_SIZE))

#define MIN_SIZE_ALIGNED RT_ALIGN(MIN_SIZE, RT_ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM RT_ALIGN(sizeof(struct rt_small_mem_item), RT_ALIGN_SIZE)

typedef struct rt_memory *rt_mem_t;
typedef rt_mem_t rt_smem_t;

static void plug_holes(struct rt_small_mem *m, struct rt_small_mem_item *mem)
{
    struct rt_small_mem_item *nmem;
    struct rt_small_mem_item *pmem;

    RT_ASSERT((__libc_uint8_t *)mem >= m->heap_ptr);
    RT_ASSERT((__libc_uint8_t *)mem < (__libc_uint8_t *)m->heap_end);

    /* plug hole forward */
    nmem = (struct rt_small_mem_item *)&m->heap_ptr[mem->next];
    if (mem != nmem && !MEM_ISUSED(nmem) &&
        (__libc_uint8_t *)nmem != (__libc_uint8_t *)m->heap_end)
    {
        /* if mem->next is unused and not end of m->heap_ptr,
         * combine mem and mem->next
         */
        if (m->lfree == nmem)
        {
            m->lfree = mem;
        }
        nmem->pool_ptr = 0;
        mem->next = nmem->next;
        ((struct rt_small_mem_item *)&m->heap_ptr[nmem->next])->prev = (__libc_uint8_t *)mem - m->heap_ptr;
    }

    /* plug hole backward */
    pmem = (struct rt_small_mem_item *)&m->heap_ptr[mem->prev];
    if (pmem != mem && !MEM_ISUSED(pmem))
    {
        /* if mem->prev is unused, combine mem and mem->prev */
        if (m->lfree == mem)
        {
            m->lfree = pmem;
        }
        mem->pool_ptr = 0;
        pmem->next = mem->next;
        ((struct rt_small_mem_item *)&m->heap_ptr[mem->next])->prev = (__libc_uint8_t *)pmem - m->heap_ptr;
    }
}

rt_smem_t rt_smem_init(const char *name,
                       void *begin_addr,
                       size_t size)
{
    struct rt_small_mem_item *mem;
    struct rt_small_mem *small_mem;
    __libc_uint32_t start_addr, begin_align, end_align, mem_size;

    small_mem = (struct rt_small_mem *)RT_ALIGN((__libc_uint32_t)begin_addr, RT_ALIGN_SIZE);
    start_addr = (__libc_uint32_t)small_mem + sizeof(*small_mem);
    begin_align = RT_ALIGN((__libc_uint32_t)start_addr, RT_ALIGN_SIZE);
    end_align = RT_ALIGN_DOWN((__libc_uint32_t)begin_addr + size, RT_ALIGN_SIZE);

    /* alignment addr */
    if ((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
        ((end_align - 2 * SIZEOF_STRUCT_MEM) >= start_addr))
    {
        /* calculate the aligned memory size */
        mem_size = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    }
    else
    {
        // printf("mem init, error begin address 0x%x, and end address 0x%x\n",
        //    (__libc_uint32_t)begin_addr, (__libc_uint32_t)begin_addr + size);

        return NULL;
    }

    memset(small_mem, 0, sizeof(*small_mem));
    /* initialize small memory object */
    small_mem->parent.algorithm = "small";
    small_mem->parent.address = begin_align;
    small_mem->parent.total = mem_size;
    small_mem->mem_size_aligned = mem_size;

    /* point to begin address of heap */
    small_mem->heap_ptr = (__libc_uint8_t *)begin_align;

    // printf("mem init, heap begin address 0x%x, size %d\n",
    //    (__libc_uint32_t)small_mem->heap_ptr, small_mem->mem_size_aligned);

    /* initialize the start of the heap */
    mem = (struct rt_small_mem_item *)small_mem->heap_ptr;
    mem->pool_ptr = MEM_FREED();
    mem->next = small_mem->mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev = 0;

    /* initialize the end of the heap */
    small_mem->heap_end = (struct rt_small_mem_item *)&small_mem->heap_ptr[mem->next];
    small_mem->heap_end->pool_ptr = MEM_USED();
    small_mem->heap_end->next = small_mem->mem_size_aligned + SIZEOF_STRUCT_MEM;
    small_mem->heap_end->prev = small_mem->mem_size_aligned + SIZEOF_STRUCT_MEM;

    /* initialize the lowest-free pointer to the start of the heap */
    small_mem->lfree = (struct rt_small_mem_item *)small_mem->heap_ptr;

    return &small_mem->parent;
}

void *rt_smem_alloc(rt_smem_t m, size_t size)
{
    size_t ptr, ptr2;
    struct rt_small_mem_item *mem, *mem2;
    struct rt_small_mem *small_mem;

    if (size == 0)
        return NULL;

    RT_ASSERT(m != NULL);

    if (size != RT_ALIGN(size, RT_ALIGN_SIZE))
    {
        // printf("malloc size %d, but align to %d\n",
        //        size, RT_ALIGN(size, RT_ALIGN_SIZE));
    }
    else
    {
        // printf("malloc size %d\n", size);
    }

    small_mem = (struct rt_small_mem *)m;
    /* alignment size */
    size = RT_ALIGN(size, RT_ALIGN_SIZE);

    /* every data block must be at least MIN_SIZE_ALIGNED long */
    if (size < MIN_SIZE_ALIGNED)
        size = MIN_SIZE_ALIGNED;

    if (size > small_mem->mem_size_aligned)
    {
        // printf("no memory\n");

        return NULL;
    }

    for (ptr = (__libc_uint8_t *)small_mem->lfree - small_mem->heap_ptr;
         ptr <= small_mem->mem_size_aligned - size;
         ptr = ((struct rt_small_mem_item *)&small_mem->heap_ptr[ptr])->next)
    {
        mem = (struct rt_small_mem_item *)&small_mem->heap_ptr[ptr];

        if ((!MEM_ISUSED(mem)) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
        {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

            if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
                (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
            {
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;

                /* create mem2 struct */
                mem2 = (struct rt_small_mem_item *)&small_mem->heap_ptr[ptr2];
                mem2->pool_ptr = MEM_FREED();
                mem2->next = mem->next;
                mem2->prev = ptr;

                /* and insert it between mem and mem->next */
                mem->next = ptr2;

                if (mem2->next != small_mem->mem_size_aligned + SIZEOF_STRUCT_MEM)
                {
                    ((struct rt_small_mem_item *)&small_mem->heap_ptr[mem2->next])->prev = ptr2;
                }
                small_mem->parent.used += (size + SIZEOF_STRUCT_MEM);
                if (small_mem->parent.max < small_mem->parent.used)
                    small_mem->parent.max = small_mem->parent.used;
            }
            else
            {
                small_mem->parent.used += mem->next - ((__libc_uint8_t *)mem - small_mem->heap_ptr);
                if (small_mem->parent.max < small_mem->parent.used)
                    small_mem->parent.max = small_mem->parent.used;
            }
            /* set small memory object */
            mem->pool_ptr = MEM_USED();

            if (mem == small_mem->lfree)
            {
                /* Find next free block after mem and update lowest free pointer */
                while (MEM_ISUSED(small_mem->lfree) && small_mem->lfree != small_mem->heap_end)
                    small_mem->lfree = (struct rt_small_mem_item *)&small_mem->heap_ptr[small_mem->lfree->next];

                RT_ASSERT(((small_mem->lfree == small_mem->heap_end) || (!MEM_ISUSED(small_mem->lfree))));
            }
            RT_ASSERT((__libc_uint32_t)mem + SIZEOF_STRUCT_MEM + size <= (rt_ubase_t)small_mem->heap_end);
            RT_ASSERT((__libc_uint32_t)((__libc_uint8_t *)mem + SIZEOF_STRUCT_MEM) % RT_ALIGN_SIZE == 0);
            RT_ASSERT((((__libc_uint32_t)mem) & (RT_ALIGN_SIZE - 1)) == 0);

            // printf("allocate memory at 0x%x, size: %d\n",
            //        (__libc_uint32_t)((__libc_uint8_t *)mem + SIZEOF_STRUCT_MEM),
            //        (__libc_uint32_t)(mem->next - ((__libc_uint8_t *)mem - small_mem->heap_ptr)));

            /* return the memory data except mem struct */
            return (__libc_uint8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }

    return NULL;
}

void rt_smem_free(void *rmem)
{
    struct rt_small_mem_item *mem;
    struct rt_small_mem *small_mem;

    if (rmem == NULL)
        return;

    RT_ASSERT((((__libc_uint32_t)rmem) & (RT_ALIGN_SIZE - 1)) == 0);

    /* Get the corresponding struct rt_small_mem_item ... */
    mem = (struct rt_small_mem_item *)((__libc_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    /* ... which has to be in a used state ... */
    small_mem = MEM_POOL(mem);
    RT_ASSERT(small_mem != RT_NULL);
    RT_ASSERT(MEM_ISUSED(mem));

    RT_ASSERT((__libc_uint8_t *)rmem >= (__libc_uint8_t *)small_mem->heap_ptr &&
              (__libc_uint8_t *)rmem < (__libc_uint8_t *)small_mem->heap_end);
    RT_ASSERT(MEM_POOL(&small_mem->heap_ptr[mem->next]) == small_mem);

    // printf("release memory 0x%x, size: %d\n",
    //        (__libc_uint32_t)rmem,
    //        (__libc_uint32_t)(mem->next - ((__libc_uint8_t *)mem - small_mem->heap_ptr)));

    /* ... and is now unused. */
    mem->pool_ptr = MEM_FREED();

    if (mem < small_mem->lfree)
    {
        /* the newly freed struct is now the lowest */
        small_mem->lfree = mem;
    }

    small_mem->parent.used -= (mem->next - ((__libc_uint8_t *)mem - small_mem->heap_ptr));

    /* finally, see if prev or next are free also */
    plug_holes(small_mem, mem);
}

void *rt_smem_realloc(rt_smem_t m, void *rmem, size_t newsize)
{
    size_t size;
    size_t ptr, ptr2;
    struct rt_small_mem_item *mem, *mem2;
    struct rt_small_mem *small_mem;
    void *nmem;

    RT_ASSERT(m != NULL);

    small_mem = (struct rt_small_mem *)m;
    /* alignment size */
    newsize = RT_ALIGN(newsize, RT_ALIGN_SIZE);
    if (newsize > small_mem->mem_size_aligned)
    {
        // printf("realloc: out of memory\n");

        return NULL;
    }
    else if (newsize == 0)
    {
        rt_smem_free(rmem);
        return NULL;
    }

    /* allocate a new memory block */
    if (rmem == NULL)
        return rt_smem_alloc(&small_mem->parent, newsize);

    RT_ASSERT((((__libc_uint32_t)rmem) & (RT_ALIGN_SIZE - 1)) == 0);
    RT_ASSERT((__libc_uint8_t *)rmem >= (__libc_uint8_t *)small_mem->heap_ptr);
    RT_ASSERT((__libc_uint8_t *)rmem < (__libc_uint8_t *)small_mem->heap_end);

    mem = (struct rt_small_mem_item *)((__libc_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    /* current memory block size */
    ptr = (__libc_uint8_t *)mem - small_mem->heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;
    if (size == newsize)
    {
        /* the size is the same as */
        return rmem;
    }

    if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size)
    {
        /* split memory block */
        small_mem->parent.used -= (size - newsize);

        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct rt_small_mem_item *)&small_mem->heap_ptr[ptr2];
        mem2->pool_ptr = MEM_FREED();
        mem2->next = mem->next;
        mem2->prev = ptr;
        mem->next = ptr2;
        if (mem2->next != small_mem->mem_size_aligned + SIZEOF_STRUCT_MEM)
        {
            ((struct rt_small_mem_item *)&small_mem->heap_ptr[mem2->next])->prev = ptr2;
        }

        if (mem2 < small_mem->lfree)
        {
            /* the splited struct is now the lowest */
            small_mem->lfree = mem2;
        }

        plug_holes(small_mem, mem2);

        return rmem;
    }

    /* expand memory */
    nmem = rt_smem_alloc(&small_mem->parent, newsize);
    if (nmem != NULL) /* check memory */
    {
        memcpy(nmem, rmem, size < newsize ? size : newsize);
        rt_smem_free(rmem);
    }

    return nmem;
}

static rt_smem_t system_heap;
void rt_system_heap_init(void *begin_addr, void *end_addr)
{
    __libc_uint32_t begin_align = RT_ALIGN((__libc_uint32_t)begin_addr, RT_ALIGN_SIZE);
    __libc_uint32_t end_align = RT_ALIGN_DOWN((__libc_uint32_t)end_addr, RT_ALIGN_SIZE);

    RT_ASSERT(end_align > begin_align);

    /* Initialize system memory heap */
    system_heap = rt_smem_init("heap", begin_addr, end_align - begin_align);
}

void *rt_malloc(size_t size)
{
    if (0 == size)
    {
        return NULL;
    }

    void *ptr;
    ptr = rt_smem_alloc(system_heap, size);
    return ptr;
}

void *rt_realloc(void *rmem, size_t newsize)
{
    void *nptr;
    nptr = rt_smem_realloc(system_heap, rmem, newsize);
    return nptr;
}

void *rt_calloc(size_t count, size_t size)
{
    void *p;

    /* allocate 'count' objects of size 'size' */
    p = rt_malloc(count * size);
    /* zero the memory */
    if (p)
    {
        memset(p, 0, count * size);
    }
    return p;
}

void rt_free(void *rmem)
{
    /* NULL check */
    if (rmem == NULL)
        return;

    rt_smem_free(rmem);
}
#endif
