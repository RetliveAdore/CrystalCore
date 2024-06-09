/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:35:36
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-02 11:26:44
 * @FilePath: \Crystal-Core\src\crmm.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalMemory.h>
#include <malloc.h>

CRAPI void* CRAlloc(void* ptr, CRUINT64 size)
{
    void* back = realloc(ptr, size);
    if (!back) return ptr;
    return back;
}