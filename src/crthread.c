/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-08 21:39:35
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-11 16:59:13
 * @FilePath: \CrystalCore\src\crthread.c
 * @Description: 
 * 多线程的简单封装
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalMemory.h>
#include <CrystalThread.h>
#include "header.h"

extern void* CRAlloc(void* ptr, CRUINT64 size);
extern CRDYNAMIC CRDyn(CRUINT64 size);
extern CRCODE CRFreeDyn(CRDYNAMIC dyn);
extern CRRBTREE CRTree(void);
extern CRCODE CRFreeTree(CRRBTREE tree);
//
extern CRCODE CRDynPush(CRDYNAMIC dyn, void* data, CRDynEnum mode);
extern CRCODE CRDynPop(CRDYNAMIC dyn, void* data, CRDynEnum mode);
extern CRCODE CRTreePut(CRRBTREE tree, CRLVOID data, CRINT64 key);
extern CRCODE CRTreeSeek(CRRBTREE tree, CRLVOID* data, CRINT64 key);
extern CRCODE CRTreeGet(CRSTRUCTURE tree, CRLVOID* data, CRINT64 key);

static CRUINT64 currentID;
static CRRBTREE threadTree;
static CRDYNAMIC availableID;

typedef struct crthread_inner
{
    CRThreadFunction func;
    CRLVOID userData;
    CRTHREAD idThis;
#ifdef CR_WINDOWS
    HWND thread;
    unsigned int threadIDw;
#elif defined CR_LINUX
    pthread_t thread;
#endif
}CRTHREADINNER, *PCRTHREADINNER;

typedef struct crlock_inner
{
#ifdef CR_WINDOWS
    CRITICAL_SECTION cs;
#elif defined CR_LINUX
    pthread_mutex_t cs;
#endif
    CRBOOL lock;
}CRLOCKINNER, *PCRLOCKINNER;

CRAPI CRCODE CRThreadInit(void)
{
    currentID = 1;
    threadTree = NULL;
    availableID = NULL;
    threadTree = CRTree();
    if (!threadTree) return 1;
    availableID = CRDyn(0);
    if (!availableID) return 2;
    return 0;
}

CRAPI CRCODE CRThreadUninit(void)
{
    if (CRFreeDyn(availableID)) return 1;
    if (CRFreeTree(threadTree)) return 2;
    return 0;
}

CRAPI void CRSleep(CRUINT64 ms)
{
    #ifdef CR_WINDOWS
    timeBeginPeriod(1);
    SleepEx((DWORD)ms, TRUE);
    timeEndPeriod(1);
    #elif defined CR_LINUX
    usleep(ms * 1000);
    #endif
}

extern void CRMemIterator(void);
static void* _inner_thread_(void* lp)
{
    PCRTHREADINNER pInner = lp;
    pInner->func(pInner->userData, pInner->idThis);
    CRTHREAD id = pInner->idThis;
    CRAlloc(lp, 0);
    CRDynPush(availableID, &id, DYN_MODE_64);
    CRTreeGet(threadTree, NULL, (CRUINT64)id);
    return 0;
}

CRAPI CRTHREAD CRThread(CRThreadFunc func, CRLVOID data)
{
    if (!func)
        return 0;
    if (!threadTree || !availableID)
        return 0;
    PCRTHREADINNER pInner = CRAlloc(NULL, sizeof(CRTHREADINNER));
    if (!pInner)
        return 0;
    if (CRDynPop(availableID, &(pInner->idThis), DYN_MODE_64))
        pInner->idThis = (CRTHREAD)currentID++;
    pInner->func = func;
    pInner->userData = data;
    CRTreePut(threadTree, pInner, (CRUINT64)(pInner->idThis));
    #ifdef CR_WINDOWS
    pInner->thread = (HWND)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)_inner_thread_, pInner, 0, &(pInner->threadIDw));
    if (pInner->thread) CloseHandle(pInner->thread);
    #elif defined CR_LINUX
    pthread_create(&(pInner->thread), NULL, _inner_thread_, pInner);
    if (pInner->thread) pthread_detach(pInner->thread);
    #endif
    return pInner->idThis;
}

CRAPI void CRWaitThread(CRTHREAD thread)
{
    while (!CRTreeSeek(threadTree, NULL, (CRUINT64)thread)) CRSleep(1);
}

CRAPI CRLOCK CRLockCreate(void)
{
    PCRLOCKINNER pInner = CRAlloc(NULL, sizeof(CRLOCKINNER));
    if (!pInner)
        return pInner;
    InitializeCriticalSection(&(pInner->cs));
    pInner->lock = CRFALSE;
    return pInner;
}

CRAPI void CRLockRelease(CRLOCK lock)
{
    PCRLOCKINNER pInner = lock;
    if (!pInner)
        return;
    while (pInner->lock) CRSleep(1);
    DeleteCriticalSection(&(pInner->cs));
    CRAlloc(pInner, 0);
}

CRAPI void CRLock(CRLOCK lock)
{
    PCRLOCKINNER pInner = lock;
    if (!pInner)
        return;
Block:
    while (pInner->lock) CRSleep(1);
    EnterCriticalSection(&(pInner->cs));
    if (pInner->lock)
    {
        LeaveCriticalSection(&(pInner->cs));
        goto Block;
    }
    pInner->lock = CRTRUE;
    LeaveCriticalSection(&(pInner->cs));
}

CRAPI void CRUnlock(CRLOCK lock)
{
    PCRLOCKINNER pInner = lock;
    if (!pInner)
        return;
    pInner->lock = CRFALSE;
}