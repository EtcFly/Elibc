#include "_stdio.h"
#include "_string.h"
#include "_stdarg.h"
#include "_ctype.h"
#include "local_console.h"

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
#define LC_MEM_PRINTF(fmt, ...) // local_printf(fmt, ##__VA_ARGS__)

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
