/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-02 17:23:23
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-10 20:47:02
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
#include <process.h>
#elif defined CR_LINUX
#include <pthread.h>
#include <unistd.h>
void InitializeCriticalSection(pthread_mutex_t* mt);
void DeleteCriticalSection(pthread_mutex_t* mt);
void EnterCriticalSection(pthread_mutex_t* mt);
void LeaveCriticalSection(pthread_mutex_t* mt);
#endif

typedef struct crstructurepub
{
    #ifdef CR_WINDOWS
    CRITICAL_SECTION cs;  //确保多线程安全
    #elif defined CR_LINUX
    pthread_mutex_t cs;  //确保多线程安全
    #endif
    CRUINT64 total;  //用于指明当前元素数量
}CRSTRUCTUREPUB;

CRCODE _inner_initialize_();
CRCODE _inner_delete_();
CRCODE _inner_crthread_init_();
CRCODE _inner_crthread_uninit_();

#endif