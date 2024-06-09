/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:36:02
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-01 23:39:24
 * @FilePath: \Crystal-Core\include\CrystalMemory.h
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTALMEMORY_H_
#define _INCLUDE_CRYSTALMEMORY_H_

#include "definitions.h"

/**
 * 用于分配内存，传入一个指针（可以是NULL），返回重新分配后的指针。
 * 如果size传0，而且传入的是非NULL指针，将执行释放内存操作。
 * 只有在分配成功后且容量不为0才会返回内存地址否则返回NULL
*/
void* CRAlloc(void* ptr, CRUINT64 size);

#endif
