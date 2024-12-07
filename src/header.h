/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-02 17:23:23
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-06 14:40:45
 * @FilePath: \CrystalCore\src\header.h
 * @Description: 
 * CrystalCore内部共享的头文件
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_HEADER_H_
#define _INCLUDE_HEADER_H_

/**
 * 用于多线程操作加锁，确保安全性
 */
#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
void InitializeCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_init(mt, NULL);
}
void DeleteCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_destroy(mt);
}
void EnterCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_lock(mt);
}
void LeaveCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_unlock(mt);
}
#endif

void _inner_initialize_();
void _inner_delete_();

#endif