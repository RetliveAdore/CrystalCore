/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-09 22:39:11
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-09 22:56:07
 * @FilePath: \CrystalCore\include\CrystalThread.h
 * @Description: 
 * 多线程相关定义
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */

#ifndef _INCLUDE_CRYSTAL_THREAD_H_
#define _INCLUDE_CRYSTAL_THREAD_H_

#include "definitions.h"

typedef void(*CRThreadFunction)(CRLVOID data, CRTHREAD idThis);
typedef CRLVOID CRLOCK;

#endif