/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:35:36
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-07 15:50:41
 * @FilePath: \CrystalCore\src\crmm.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalMemory.h>
#include <CrystalLog.h>
#include <malloc.h>
#include "header.h"

static unsigned char* ram = NULL;
static CRUINT64 ramSize = 0;
#ifdef CR_WINDOWS
static CRITICAL_SECTION mem_cs;  //确保多线程安全
#elif defined CR_LINUX
static pthread_mutex_t mem_cs;  //确保多线程安全
#endif

//prev、this、next存储的都是在ram中的下标
typedef struct block_header
{
    CRINT64 prev;
    CRINT64 this;
    CRINT64 next;
    //1: used, 0: available, 2: endblock
    CRUINT64 used;
    CRUINT64 size;
    CRUINT64 reserved;
} *PBLOCK_HEADER;
#define HEADER_SIZE sizeof(struct block_header)

void _inner_initialize_()
{
    InitializeCriticalSection(&mem_cs);
}

void _inner_delete_()
{
    DeleteCriticalSection(&mem_cs);
}

/**
 * 内部函数，获取前一个块头
 */
static inline PBLOCK_HEADER _inner_get_prev_block_(PBLOCK_HEADER header)
{
    if (header->prev != -1) return (PBLOCK_HEADER)&(ram[header->prev]);
    return NULL;
}
/**
 * 内部函数，获取后一个块头
 */
static inline PBLOCK_HEADER _inner_get_next_block_(PBLOCK_HEADER header)
{
    if (header->next) return (PBLOCK_HEADER)&(ram[header->next]);
    return NULL;
}
/**
 * 内部内联函数，负责创建动态内存堆的初始块和结尾块。
 */
static inline void _inner_create_initial_blocks_(CRUINT64 size)
{
    PBLOCK_HEADER mem1 = (PBLOCK_HEADER)ram;
    PBLOCK_HEADER mem2 = NULL;
    mem1->prev = -1;
    mem1->this = 0;
    mem1->next = size - HEADER_SIZE;
    mem1->size = size - HEADER_SIZE * 2;
    mem1->used = 0;
    //
    mem2 = _inner_get_next_block_(mem1);
    mem2->prev = mem1->this;
    mem2->this = mem1->next;
    mem2->next = 0;
    mem2->size = 0;
    mem2->used = 2;
}

/**
 * 初始化动态内存堆
 * 传入初始化的内存字节数，
 * 初始化之后，在没有分配内存的情况下，可以重新初始化。
 * 程序结束之后需要执行CRMemClear来释放内存堆。
 * 返回值：
 * 0：初始化成功；
 * 1：内存申请失败；
 * 2：正在使用中的内存。
 * 3：传入的容量（size）过小
 */
CRAPI CRCODE CRMemSetup(CRUINT64 size)
{
    //数据头的容量是包含在总容量中的。如果size太小将无法创建
    if (size <= sizeof(struct block_header) * 2)
        return 3;
    EnterCriticalSection(&mem_cs);
    PBLOCK_HEADER mem = (PBLOCK_HEADER)ram;
    if (!ram)
    {
        ram = malloc(size);
        if (!ram)
        {
            LeaveCriticalSection(&mem_cs);
            return 1;
        }
        //创建头尾块。
        _inner_create_initial_blocks_(size);
        ramSize = size;
        LeaveCriticalSection(&mem_cs);
        return 0;
    }
    else //如果内存堆里面有尚未释放的块，就返回2，否则重新分配内存堆大小
    {
        while (mem->next)  //只要next不为空，则不为结尾块（结尾块used恒为CRTRUE）
        {
            if (mem->used)
            {
                LeaveCriticalSection(&mem_cs);
                return 2;
            }
            mem = _inner_get_next_block_(mem);
        };
        ram = realloc(ram, size);
        if (!ram)
        {
            LeaveCriticalSection(&mem_cs);
            return 1;
        }
        //创建头尾块。
        _inner_create_initial_blocks_(size);
        ramSize = size;
    }
    LeaveCriticalSection(&mem_cs);
    return 0;
}

/**
 * 释放动态内存堆
 * 此时无论是否有正在使用的内存块都将直接释放堆，请确保在退出的最后一步执行。
 * 释放之后可以使用CRMemSetup重新初始化内存堆
 * 返回值：
 * 0：正常释放；
 * 1：有正在使用的块（仍然释放）；
 * 2：尚未初始化。
 */
CRAPI CRCODE CRMemClear(void)
{
    CRCODE back = 0;
    PBLOCK_HEADER mem = (PBLOCK_HEADER)ram;
    if (!ram) return 2;
    //检查是否有尚未释放的块
    //即使有尚未释放的块，也将强制释放，但返回1。
    EnterCriticalSection(&mem_cs);
    while (mem->next)
    {
        if (mem->used)
        {
            back = 1;
            break;
        }
        mem = _inner_get_next_block_(mem);
    }
    free(ram);
    ram = NULL;
    LeaveCriticalSection(&mem_cs);
    return back;
}

extern CRAPI CRINT64 CRPrint(CRTextColor color, const CRCHAR* fmt, ...);
/**
 * 内部函数，用于分配内存块时分割空闲块
 */
static void _inner_split_block_(PBLOCK_HEADER header, CRUINT64 size)
{
    header->used = 1;
    if (header->size > size + HEADER_SIZE)
    {
        PBLOCK_HEADER newHeader = (PBLOCK_HEADER)&ram[header->this + size + HEADER_SIZE];
        newHeader->prev = header->this;
        newHeader->this = header->this + size + HEADER_SIZE;
        newHeader->next = header->next;
        newHeader->used = 0;
        newHeader->size = header->size - size - HEADER_SIZE;
        //
        header->next = newHeader->this;
        header->size = size;
        //
        header = _inner_get_next_block_(newHeader);
        header->prev = newHeader->this;
    }
}
/**
 * 内部函数，用于寻找合适的块然后分配内存
 */
static PBLOCK_HEADER _inner_allocate_(CRUINT64 size)
{
    PBLOCK_HEADER header = (PBLOCK_HEADER)ram;
    while (header->next)
    {
        if (!header->used && header->size >= size)
        {
            _inner_split_block_(header, size);
            return header;
        }
        header = _inner_get_next_block_(header);
    }
    return NULL;
}
/**
 * 内部函数，用于释放并融合内存块
 */
static PBLOCK_HEADER _inner_melt_(PBLOCK_HEADER header)
{
    header->used = 0;
    PBLOCK_HEADER p, n;
Repeat:  //理论上是不需要重复的，但为了防止意外情况导致内存分裂，进行迭代操作
    p = _inner_get_prev_block_(header);
    n = _inner_get_next_block_(header);
    if (n && !n->used)
    {
        header->size += n->size + HEADER_SIZE;
        header->next = n->next;
        n = _inner_get_next_block_(header);
        n->prev = header->this;
        goto Repeat;
    }
    if (p && !p->used)
    {
        header = p;
        goto Repeat;
    }
    return header;
}
/**
 * 内部函数，用于指针转换为块头
 */
static inline PBLOCK_HEADER _inner_ptr_to_header_(void* ptr)
{
    return (PBLOCK_HEADER)((CRUINT64)ptr - (CRUINT64)HEADER_SIZE);
}
/**
 * 内部函数，用于块头转换为指针
 */
static inline void* _inner_header_to_ptr_(PBLOCK_HEADER header)
{
    return (void*)((CRUINT64)header + (CRUINT64)HEADER_SIZE);
}
/**
 * 用于分配内存，传入一个指针（可以是NULL），返回重新分配后的指针。
 * 如果size传0，而且传入的是非NULL指针，将执行释放内存操作。
 * 只有在分配成功后且容量不为0才会返回内存地址否则返回NULL
*/
CRAPI void* CRAlloc(void* ptr, CRUINT64 size)
{
    if (!ram) return NULL;
    EnterCriticalSection(&mem_cs);
    if (!ptr)
    {
        if (!size)
        {
            LeaveCriticalSection(&mem_cs);
            return NULL;
        }
        PBLOCK_HEADER header = _inner_allocate_(size);
        if (header)
        {
            LeaveCriticalSection(&mem_cs);
            return _inner_header_to_ptr_(header);
        }
    }
    else if (!size)  //释放内存
    {
        PBLOCK_HEADER header = _inner_ptr_to_header_(ptr);
        _inner_melt_(header);
    }
    else  //调整内存
    {
        PBLOCK_HEADER headerOld = _inner_ptr_to_header_(ptr);
        PBLOCK_HEADER header = _inner_melt_(headerOld);
        if (size < header->size)
        {
            _inner_split_block_(header, size);
            if (header == headerOld)
            {
                LeaveCriticalSection(&mem_cs);
                return ptr;
            }
        }
        else if (size > header->size)
        {
            header = _inner_allocate_(size);
            if (!header)
            {
                LeaveCriticalSection(&mem_cs);
                return NULL;
            }
        }
        else
        {
            LeaveCriticalSection(&mem_cs);
            return ptr;
        }
        //拷贝内存
        unsigned char* mem = _inner_header_to_ptr_(header);
        for (CRUINT64 i = 0; i < headerOld->size; i++)
            mem[i] = ((unsigned char*)ptr)[i];
        LeaveCriticalSection(&mem_cs);
        return mem;
    }
    LeaveCriticalSection(&mem_cs);
    return NULL;
}

/**
 * 遍历动态内存堆
 * 将所有正在使用的内存块一一列出，仅提供显示，无其他操作。
 * 返回值：无
 */
CRAPI void CRMemIterator(void)
{
    PBLOCK_HEADER block = (PBLOCK_HEADER)ram;
    CRUINT64 i = 1;
    EnterCriticalSection(&mem_cs);
    while (block->next)
    {
        CRPrint(CR_TC_BLUE,
            "block%d:\tusage: %d, size: %d,\tp: %d,\tn: %d,\tthis: %d\n",
            i, block->used, block->size,
            block->prev, block->next, block->this
        );
        block = _inner_get_next_block_(block);
        i++;
    }
    CRPrint(CR_TC_BLUE,
        "block%d:\tusage: %d, size: %d,\tp: %d,\tn: %d,\tthis: %d\n\n",
        i, block->used, block->size,
        block->prev, block->next, block->this
    );
    LeaveCriticalSection(&mem_cs);
}