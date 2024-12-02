/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:35:36
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-02 23:09:26
 * @FilePath: \CrystalCore\src\crmm.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalMemory.h>
#include <CrystalLog.h>
#include <malloc.h>

static unsigned char* ram = NULL;
static CRUINT64 ramSize = 0;

typedef struct block_header
{
    struct block_header* prev;
    //if next if NULL, it's end of ram.
    struct block_header* next;
    //CRTRUE: used, CRFALSE: available
    CRBOOL used;
    CRUINT64 size;
} *PBLOCK_HEADER;

/**
 * 内部内联函数，负责创建动态内存堆的初始块和结尾块。
 */
static inline void _inner_create_initial_blocks_(CRUINT64 size)
{
    PBLOCK_HEADER mem1 = (PBLOCK_HEADER)ram;
    PBLOCK_HEADER mem2 = NULL;
    mem1->prev = NULL;
    mem1->next = (PBLOCK_HEADER)&(ram[size - sizeof(struct block_header) - 1]);
    mem1->used = CRFALSE;
    mem1->size = size - sizeof(struct block_header) * 2;
    mem2 = mem1->next;
    mem2->prev = mem1;
    mem2->next = NULL;
    mem2->size = 0;
    //结尾块始终处于已分配状态，此类为特殊情况
    mem2->used = CRTRUE;
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
    PBLOCK_HEADER mem = (PBLOCK_HEADER)ram;
    if (!ram)
    {
        ram = malloc(size);
        if (!ram)
            return 1;
        //创建头尾块。
        _inner_create_initial_blocks_(size);
        ramSize = size;
        return 0;
    }
    else //如果内存堆里面有尚未释放的块，就返回2，否则重新分配内存堆大小
    {
        while (mem->next)  //只要next不为空，则不为结尾块（结尾块used恒为CRTRUE）
        {
            if (mem->used)
                return 2;
            mem = mem->next;
        };
        ram = realloc(ram, size);
        if (!ram)
            return 1;
        //创建头尾块。
        _inner_create_initial_blocks_(size);
        ramSize = size;
    }
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
CRCODE CRMemClear(void)
{
    CRCODE back = 0;
    PBLOCK_HEADER mem1 = (PBLOCK_HEADER)ram;
    PBLOCK_HEADER mem2 = NULL;
    if (!ram) return 2;
    //检查是否有尚未释放的块
    //即使有尚未释放的块，也将强制释放，但返回1。
    while (mem1->next)
    {
        mem2 = mem1->next;
        if (mem2->used && mem2->next)
            back = 1;
            mem1 = mem2;
    }
    free(ram);
    ram = NULL;
    return back;
}

/**
 * 用于分配内存，传入一个指针（可以是NULL），返回重新分配后的指针。
 * 如果size传0，而且传入的是非NULL指针，将执行释放内存操作。
 * 只有在分配成功后且容量不为0才会返回内存地址否则返回NULL
*/
CRAPI void* CRAlloc(void* ptr, CRUINT64 size)
{
    void* back = realloc(ptr, size);
    if (!back) return ptr;
    return back;
}

extern CRAPI CRINT64 CRPrint(CRTextColor color, const CRCHAR* fmt, ...);
/**
 * 遍历动态内存堆
 * 将所有正在使用的内存块一一列出，仅提供显示，无其他操作。
 * 返回值：无
 */
CRAPI void CRMemIterator(void)
{
    PBLOCK_HEADER block = (PBLOCK_HEADER)ram;
    if (ram)
    {
        CRUINT64 i = 1;
        while(block->next)
        {
            CRPrint(CR_TC_BLUE, "block%d:\t%s,\tsize: %d\n",
                i,
                block->used ? "used" : "available",
                block->size
            );
            i++;
            block = block->next;
        }
        CRPrint(CR_TC_BLUE, "block%d:\tendblock\n", i);
    }
    else CRPrint(CR_TC_RED, "ram not inited!\n");
}