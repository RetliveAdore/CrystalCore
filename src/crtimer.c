/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-05 12:46:38
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-05 16:49:35
 * @FilePath: \CrystalCore\src\crtimer.c
 * @Description: 
 * 计时器实现相关
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include "header.h"

#ifdef CR_WINDOWS
#include <Windows.h>
LARGE_INTEGER frequency = {0};
LARGE_INTEGER count = {0};
//
#elif defined CR_LINUX
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
static time_t crTime = 0;
static struct tm* ptm = NULL;
static struct timeval ti = { 0 };
static struct timespec ts = { 0 };
#endif

CRCODE _inner_timer_setup_()
{
#ifdef CR_WINDOWS
    if (!QueryPerformanceFrequency(&frequency)) return 1;
#endif
    return 0;
}

CRAPI double CRTimerMark(CRTIMER *pTimer)
{
	double now = 0;
#ifdef CR_WINDOWS
	QueryPerformanceCounter(&count);
	now = (double)(count.QuadPart) / (double)(frequency.QuadPart);
#elif defined CR_LINUX
	clock_gettime(CLOCK_MONOTONIC, &ts);
	now = (double)(ts.tv_sec) + (double)(ts.tv_nsec) / 1000000000;
#endif
	double back = now - *pTimer;
	*pTimer = now;
	return back;
}

CRAPI double CRTimerPeek(CRTIMER *pTimer)
{
#ifdef CR_WINDOWS
	QueryPerformanceCounter(&count);
	return (double)(count.QuadPart) / (double)(frequency.QuadPart) - *pTimer;
#elif defined CR_LINUX
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)(ts.tv_sec) + (double)(ts.tv_nsec) / 1000000000 - *pTimer;
#endif
}