/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:36:02
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-10 10:27:47
 * @FilePath: \CrystalCore\include\CrystalMemory.h
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTALMEMORY_H_
#define _INCLUDE_CRYSTALMEMORY_H_

#include "definitions.h"

typedef CRLVOID CRDYNAMIC;
typedef CRLVOID CRRBTREE;

typedef void(*IteratorCallback)(CRLVOID data, CRLVOID user, CRUINT64 key);

typedef enum CRDynEnum
{
    DYN_MODE_8 = 0,
    DYN_MODE_16 = 1,
    DYN_MODE_32 = 2,
    DYN_MODE_64 = 3
}CRDynEnum;

#endif
