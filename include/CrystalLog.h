/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-05-29 18:20:55
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-02 11:37:56
 * @FilePath: \Crystal-Core\include\CrystalLog.h
 * @Description: 日志系统
 * 包含但不限于一般日志系统
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTALLOG_H_
#define _INCLUDE_CRYSTALLOG_H_

#include "definitions.h"

typedef enum CRTextColor
{
    CR_TC_BLACK = 0,
    CR_TC_BLUE = 1,
    CR_TC_GREEN = 2,
    CR_TC_CYAN = 3,
    CR_TC_RED = 4,
    CR_TC_MAGENTA = 5,
    CR_TC_BROWN = 6,
    CR_TC_LIGHTGRAY = 7,
    CR_TC_DARKGRAY = 8,
    CR_TC_LIGHTBLUE = 9,
    CR_TC_LIGHTGREEN = 10,
    CR_TC_LIGHTCYAN = 11,
    CR_TC_LIGHTRED = 12,
    CR_TC_LIGHTMAGENTA = 13,
    CR_TC_YELLOW = 14,
    CR_TC_WHITE = 15
} CRTextColor;

typedef struct CRDate
{
    CRINT16 year;
	CRINT16 month;
	CRINT16 dweek;
	CRINT16 day;
	CRINT16 hour;
	CRINT16 min;
	CRINT16 sec;
	CRINT16 ms;
}CRDate;

#endif
