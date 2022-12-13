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

int local_printf(const char *__restrict format, ...)
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

struct memory_info
{
    const char *algorithm;   /**< Memory management algorithm name */
    __libc_uint32_t address; /**< memory start address */
    size_t total;            /**< memory size */
    size_t used;             /**< size used */
    size_t max;              /**< maximum usage */
};

struct mem_block_item
{
    __libc_uint32_t pool_ptr; /**< small memory object addr */
    size_t next;              /**< next free item */
    size_t prev;              /**< prev free item */
};

struct mem_block
{
    struct memory_info parent; /**< inherit from memory_info */
    __libc_uint8_t *heap_ptr;  /**< pointer to the heap */
    struct mem_block_item *heap_end;
    struct mem_block_item *lfree;
    size_t mem_size_aligned; /**< aligned memory size */
};

#define MEM_MIN_SIZE 12
#define MEM_ALIGN_SIZE 4
#define LC_MEM_ASSERT(...)
#define LC_MEM_PRINTF(fmt, ...) local_printf(fmt, ##__VA_ARGS__)

#define MEM_MASK 0xfffffffe
#define MEM_USED() ((((__libc_uint32_t)(pblock_mem)) & MEM_MASK) | 0x1)
#define MEM_FREED() ((((__libc_uint32_t)(pblock_mem)) & MEM_MASK) | 0x0)

#define MEM_ISUSED(_mem) \
    (((__libc_uint32_t)(((struct mem_block_item *)(_mem))->pool_ptr)) & (~MEM_MASK))
#define MEM_POOL(_mem) \
    ((struct mem_block *)(((__libc_uint32_t)(((struct mem_block_item *)(_mem))->pool_ptr)) & (MEM_MASK)))

#define LC_MEM_ALIGN(size, align) (((size) + (align)-1) & ~((align)-1))
#define LC_MEM_ALIGN_DOWN(size, align) ((size) & ~((align)-1))
#define MEM_SIZE(_heap, _mem) \
    (((struct mem_block_item *)(_mem))->next - ((__libc_uint32_t)(_mem) - (__libc_uint32_t)((_heap)->heap_ptr)) - LC_MEM_ALIGN(sizeof(struct mem_block_item), MEM_ALIGN_SIZE))

#define MIN_SIZE_ALIGNED LC_MEM_ALIGN(MEM_MIN_SIZE, MEM_ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM LC_MEM_ALIGN(sizeof(struct mem_block_item), MEM_ALIGN_SIZE)

typedef struct memory_info *lc_mem_info_t;
typedef lc_mem_info_t plc_mem_t;

static void plug_holes(struct mem_block *m, struct mem_block_item *mem)
{
    struct mem_block_item *nmem;
    struct mem_block_item *pmem;

    LC_MEM_ASSERT((__libc_uint8_t *)mem >= m->heap_ptr);
    LC_MEM_ASSERT((__libc_uint8_t *)mem < (__libc_uint8_t *)m->heap_end);

    nmem = (struct mem_block_item *)&m->heap_ptr[mem->next];
    if (mem != nmem && !MEM_ISUSED(nmem) &&
        (__libc_uint8_t *)nmem != (__libc_uint8_t *)m->heap_end)
    {
        if (m->lfree == nmem)
        {
            m->lfree = mem;
        }
        nmem->pool_ptr = 0;
        mem->next = nmem->next;
        ((struct mem_block_item *)&m->heap_ptr[nmem->next])->prev = (__libc_uint8_t *)mem - m->heap_ptr;
    }

    pmem = (struct mem_block_item *)&m->heap_ptr[mem->prev];
    if (pmem != mem && !MEM_ISUSED(pmem))
    {
        if (m->lfree == mem)
        {
            m->lfree = pmem;
        }
        mem->pool_ptr = 0;
        pmem->next = mem->next;
        ((struct mem_block_item *)&m->heap_ptr[mem->next])->prev = (__libc_uint8_t *)pmem - m->heap_ptr;
    }
}

plc_mem_t mem_block_init(const char *name,
                         void *begin_addr,
                         size_t size)
{
    struct mem_block_item *mem;
    struct mem_block *pblock_mem;
    __libc_uint32_t start_addr, begin_align, end_align, mem_size;

    pblock_mem = (struct mem_block *)LC_MEM_ALIGN((__libc_uint32_t)begin_addr, MEM_ALIGN_SIZE);
    start_addr = (__libc_uint32_t)pblock_mem + sizeof(*pblock_mem);
    begin_align = LC_MEM_ALIGN((__libc_uint32_t)start_addr, MEM_ALIGN_SIZE);
    end_align = LC_MEM_ALIGN_DOWN((__libc_uint32_t)begin_addr + size, MEM_ALIGN_SIZE);

    if ((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
        ((end_align - 2 * SIZEOF_STRUCT_MEM) >= start_addr))
    {
        mem_size = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    }
    else
    {
        LC_MEM_PRINTF("mem init, error begin address 0x%x, and end address 0x%x\r\n",
                      (__libc_uint32_t)begin_addr, (__libc_uint32_t)begin_addr + size);

        return NULL;
    }

    memset(pblock_mem, 0, sizeof(*pblock_mem));
    pblock_mem->parent.algorithm = "small";
    pblock_mem->parent.address = begin_align;
    pblock_mem->parent.total = mem_size;
    pblock_mem->mem_size_aligned = mem_size;

    pblock_mem->heap_ptr = (__libc_uint8_t *)begin_align;

    LC_MEM_PRINTF("mem init, heap begin address 0x%x, size %d\r\n",
                  (__libc_uint32_t)pblock_mem->heap_ptr, pblock_mem->mem_size_aligned);

    mem = (struct mem_block_item *)pblock_mem->heap_ptr;
    mem->pool_ptr = MEM_FREED();
    mem->next = pblock_mem->mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev = 0;

    pblock_mem->heap_end = (struct mem_block_item *)&pblock_mem->heap_ptr[mem->next];
    pblock_mem->heap_end->pool_ptr = MEM_USED();
    pblock_mem->heap_end->next = pblock_mem->mem_size_aligned + SIZEOF_STRUCT_MEM;
    pblock_mem->heap_end->prev = pblock_mem->mem_size_aligned + SIZEOF_STRUCT_MEM;

    pblock_mem->lfree = (struct mem_block_item *)pblock_mem->heap_ptr;
    return &pblock_mem->parent;
}

void *mem_alloc(plc_mem_t m, size_t size)
{
    size_t ptr, ptr2;
    struct mem_block_item *mem, *mem2;
    struct mem_block *pblock_mem;

    if (size == 0)
        return NULL;

    LC_MEM_ASSERT(m != NULL);

    if (size != LC_MEM_ALIGN(size, MEM_ALIGN_SIZE))
    {
        LC_MEM_PRINTF("malloc size %d, but align to %d\r\n",
                      size, LC_MEM_ALIGN(size, MEM_ALIGN_SIZE));
    }
    else
    {
        LC_MEM_PRINTF("malloc size %d\r\n", size);
    }

    pblock_mem = (struct mem_block *)m;
    size = LC_MEM_ALIGN(size, MEM_ALIGN_SIZE);

    if (size < MIN_SIZE_ALIGNED)
        size = MIN_SIZE_ALIGNED;

    if (size > pblock_mem->mem_size_aligned)
    {
        LC_MEM_PRINTF("no memory\r\n");

        return NULL;
    }

    for (ptr = (__libc_uint8_t *)pblock_mem->lfree - pblock_mem->heap_ptr;
         ptr <= pblock_mem->mem_size_aligned - size;
         ptr = ((struct mem_block_item *)&pblock_mem->heap_ptr[ptr])->next)
    {
        mem = (struct mem_block_item *)&pblock_mem->heap_ptr[ptr];

        if ((!MEM_ISUSED(mem)) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
        {
            if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
                (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
            {
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;

                mem2 = (struct mem_block_item *)&pblock_mem->heap_ptr[ptr2];
                mem2->pool_ptr = MEM_FREED();
                mem2->next = mem->next;
                mem2->prev = ptr;
                mem->next = ptr2;

                if (mem2->next != pblock_mem->mem_size_aligned + SIZEOF_STRUCT_MEM)
                {
                    ((struct mem_block_item *)&pblock_mem->heap_ptr[mem2->next])->prev = ptr2;
                }
                pblock_mem->parent.used += (size + SIZEOF_STRUCT_MEM);
                if (pblock_mem->parent.max < pblock_mem->parent.used)
                    pblock_mem->parent.max = pblock_mem->parent.used;
            }
            else
            {
                pblock_mem->parent.used += mem->next - ((__libc_uint8_t *)mem - pblock_mem->heap_ptr);
                if (pblock_mem->parent.max < pblock_mem->parent.used)
                    pblock_mem->parent.max = pblock_mem->parent.used;
            }

            mem->pool_ptr = MEM_USED();
            if (mem == pblock_mem->lfree)
            {
                while (MEM_ISUSED(pblock_mem->lfree) && pblock_mem->lfree != pblock_mem->heap_end)
                    pblock_mem->lfree = (struct mem_block_item *)&pblock_mem->heap_ptr[pblock_mem->lfree->next];

                LC_MEM_ASSERT(((pblock_mem->lfree == pblock_mem->heap_end) || (!MEM_ISUSED(pblock_mem->lfree))));
            }
            LC_MEM_ASSERT((__libc_uint32_t)mem + SIZEOF_STRUCT_MEM + size <= (rt_ubase_t)pblock_mem->heap_end);
            LC_MEM_ASSERT((__libc_uint32_t)((__libc_uint8_t *)mem + SIZEOF_STRUCT_MEM) % MEM_ALIGN_SIZE == 0);
            LC_MEM_ASSERT((((__libc_uint32_t)mem) & (MEM_ALIGN_SIZE - 1)) == 0);

            LC_MEM_PRINTF("allocate memory at 0x%x, size: %d\r\n",
                          (__libc_uint32_t)((__libc_uint8_t *)mem + SIZEOF_STRUCT_MEM),
                          (__libc_uint32_t)(mem->next - ((__libc_uint8_t *)mem - pblock_mem->heap_ptr)));

            return (__libc_uint8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }

    return NULL;
}

void mem_free(void *rmem)
{
    struct mem_block_item *mem;
    struct mem_block *pblock_mem;

    if (rmem == NULL)
        return;

    LC_MEM_ASSERT((((__libc_uint32_t)rmem) & (MEM_ALIGN_SIZE - 1)) == 0);
    mem = (struct mem_block_item *)((__libc_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    pblock_mem = MEM_POOL(mem);
    LC_MEM_ASSERT(pblock_mem != RT_NULL);
    LC_MEM_ASSERT(MEM_ISUSED(mem));

    LC_MEM_ASSERT((__libc_uint8_t *)rmem >= (__libc_uint8_t *)pblock_mem->heap_ptr &&
                  (__libc_uint8_t *)rmem < (__libc_uint8_t *)pblock_mem->heap_end);
    LC_MEM_ASSERT(MEM_POOL(&pblock_mem->heap_ptr[mem->next]) == pblock_mem);

    LC_MEM_PRINTF("release memory 0x%x, size: %d\r\n",
                  (__libc_uint32_t)rmem,
                  (__libc_uint32_t)(mem->next - ((__libc_uint8_t *)mem - pblock_mem->heap_ptr)));

    mem->pool_ptr = MEM_FREED();

    if (mem < pblock_mem->lfree)
    {
        pblock_mem->lfree = mem;
    }

    pblock_mem->parent.used -= (mem->next - ((__libc_uint8_t *)mem - pblock_mem->heap_ptr));
    plug_holes(pblock_mem, mem);
}

void *mem_realloc(plc_mem_t m, void *rmem, size_t newsize)
{
    size_t size;
    size_t ptr, ptr2;
    struct mem_block_item *mem, *mem2;
    struct mem_block *pblock_mem;
    void *nmem;

    LC_MEM_ASSERT(m != NULL);

    pblock_mem = (struct mem_block *)m;
    newsize = LC_MEM_ALIGN(newsize, MEM_ALIGN_SIZE);
    if (newsize > pblock_mem->mem_size_aligned)
    {
        LC_MEM_PRINTF("realloc: out of memory\n");
        return NULL;
    }
    else if (newsize == 0)
    {
        mem_free(rmem);
        return NULL;
    }

    if (rmem == NULL)
        return mem_alloc(&pblock_mem->parent, newsize);

    LC_MEM_ASSERT((((__libc_uint32_t)rmem) & (MEM_ALIGN_SIZE - 1)) == 0);
    LC_MEM_ASSERT((__libc_uint8_t *)rmem >= (__libc_uint8_t *)pblock_mem->heap_ptr);
    LC_MEM_ASSERT((__libc_uint8_t *)rmem < (__libc_uint8_t *)pblock_mem->heap_end);

    mem = (struct mem_block_item *)((__libc_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    ptr = (__libc_uint8_t *)mem - pblock_mem->heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;
    if (size == newsize)
    {
        return rmem;
    }

    if (newsize + SIZEOF_STRUCT_MEM + MEM_MIN_SIZE < size)
    {
        pblock_mem->parent.used -= (size - newsize);

        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct mem_block_item *)&pblock_mem->heap_ptr[ptr2];
        mem2->pool_ptr = MEM_FREED();
        mem2->next = mem->next;
        mem2->prev = ptr;
        mem->next = ptr2;
        if (mem2->next != pblock_mem->mem_size_aligned + SIZEOF_STRUCT_MEM)
        {
            ((struct mem_block_item *)&pblock_mem->heap_ptr[mem2->next])->prev = ptr2;
        }

        if (mem2 < pblock_mem->lfree)
        {
            pblock_mem->lfree = mem2;
        }

        plug_holes(pblock_mem, mem2);

        return rmem;
    }

    nmem = mem_alloc(&pblock_mem->parent, newsize);
    if (nmem != NULL)
    {
        memcpy(nmem, rmem, size < newsize ? size : newsize);
        mem_free(rmem);
    }

    return nmem;
}

static plc_mem_t system_heap;
void mem_heap_init(void *begin_addr, void *end_addr)
{
    __libc_uint32_t begin_align = LC_MEM_ALIGN((__libc_uint32_t)begin_addr, MEM_ALIGN_SIZE);
    __libc_uint32_t end_align = LC_MEM_ALIGN_DOWN((__libc_uint32_t)end_addr, MEM_ALIGN_SIZE);

    LC_MEM_ASSERT(end_align > begin_align);
    system_heap = mem_block_init("heap", begin_addr, end_align - begin_align);
}

void *malloc(size_t size)
{
    if (0 == size)
    {
        return NULL;
    }

    void *ptr;
    ptr = mem_alloc(system_heap, size);
    return ptr;
}

void *realloc(void *rmem, size_t newsize)
{
    void *nptr;
    nptr = mem_realloc(system_heap, rmem, newsize);
    return nptr;
}

void *calloc(size_t count, size_t size)
{
    void *p;

    p = malloc(count * size);
    if (p)
    {
        memset(p, 0, count * size);
    }
    return p;
}

void free(void *rmem)
{
    if (rmem == NULL)
        return;

    mem_free(rmem);
}
