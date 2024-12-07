/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-16 15:52:23
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-16 16:42:37
 * @FilePath: \CrystalCore\src\init.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalCore.h>
#include <stdio.h>
#include "header.h"

CRAPI CRBOOL CRModInit(void** list)
{
    _inner_initialize_();
    return CRTRUE;
}

CRAPI void CRModUninit(void)
{
    _inner_delete_;
}