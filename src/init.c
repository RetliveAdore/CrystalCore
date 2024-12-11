/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-16 15:52:23
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-11 11:17:42
 * @FilePath: \CrystalCore\src\init.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalCore.h>
#include <stdio.h>
#include "header.h"

#ifdef CR_LINUX
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

CRAPI CRCODE CRModInit(void** list)
{
    if (_inner_initialize_()) return 1;
    return 0;
}

CRAPI CRCODE CRModUninit(void)
{
    if(_inner_delete_()) return 1;
	return 0;
}