/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-05 15:12:25
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-07 14:39:45
 * @FilePath: \CrystalCore\src\crdyn.c
 * @Description: 
 * 动态数组的实现文件，为了禁用掉原生内存下标访问的方式，
 * 需要提供一个新的接口管理内存
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalMemory.h>
#include <string.h>
#include "header.h"

extern void* CRAlloc(void* ptr, CRUINT64 size);

typedef struct crstructurepub
{
    #ifdef CR_WINDOWS
    CRITICAL_SECTION cs;  //确保多线程安全
    #elif defined CR_LINUX
    pthread_mutex_t cs;  //确保多线程安全
    #endif
    CRUINT64 total;  //用于指明当前元素数量
}CRSTRUCTUREPUB;

typedef struct dyn
{
    CRSTRUCTUREPUB pub;
    union
	{
		CRUINT64* p64;
		CRUINT32* p32;
		CRUINT16* p16;
		CRUINT8* p8;
		CRUINT8* arr;
	};
    CRUINT32 capacity;
}CRDYN, * PCRDYN;

CRAPI CRDYNAMIC CRDyn(CRUINT64 size)
{
    CRUINT64 capacity = 8;
    size += 8 - size % 8;
    while (capacity < size) capacity <<= 2;
    PCRDYN pInner = CRAlloc(NULL, sizeof(CRDYN));
    if (!pInner)
        return NULL;
    pInner->arr = CRAlloc(NULL, capacity);
    if (!pInner->arr)
    {
        CRAlloc(pInner, 0);
        return NULL;
    }
    pInner->pub.total = 0;
    InitializeCriticalSection(&(pInner->pub.cs));
    pInner->arr[0] = 0;
    pInner->capacity = capacity;
    return pInner;
}

CRAPI CRBOOL CRFreeDyn(CRDYNAMIC dyn)
{
	PCRDYN pInner = dyn;
	if (!pInner) return CRFALSE;
	CRAlloc(pInner->arr, 0);
	CRAlloc(pInner, 0);
	return CRTRUE;
}

CRAPI CRUINT64 CRDynSize(CRDYNAMIC dyn)
{
    PCRDYN pInner = dyn;
    if (!pInner) return 0;
    return pInner->pub.total;
}

static CRUINT8* _inner_dyn_sizeup_(CRUINT8* arr, CRUINT64 size, CRUINT64 capacity)
{
    CRUINT8* tmp = CRAlloc(NULL, capacity);
    if (!tmp) return NULL;
    for (int i = 0; i < capacity; i++) tmp[i] = 0;
    memcpy(tmp, arr, size);
    CRAlloc(arr, 0);
    return tmp;
}

static CRUINT8* _inner_dyn_sizedown_(CRUINT8* arr, CRUINT64 capacity)
{
    return CRAlloc(arr, capacity);
}

static CRBOOL _inner_dyn_push_(CRDYNAMIC dyn, void* data, CRDynEnum mode)
{
	PCRDYN pInner = dyn;
	//需要扩容的情况
	if (pInner->pub.total >= pInner->capacity)
	{
		pInner->capacity <<= 1;
		CRUINT8* tmp = _inner_dyn_sizeup_(pInner->arr, pInner->pub.total, pInner->capacity);
		if (!tmp)
			return CRFALSE;
		pInner->arr = tmp;
	}
	else  //扩容之后回归正常流程
	{
		switch (mode)
		{
		case DYN_MODE_8:
			pInner->p8[pInner->pub.total++] = *(CRUINT8*)data;
			break;
		case DYN_MODE_16:
			pInner->pub.total >>= 1;
			pInner->pub.total <<= 1;
			pInner->p16[pInner->pub.total] = *(CRUINT16*)data;
			pInner->pub.total += 2;
			break;
		case DYN_MODE_32:
			pInner->pub.total >>= 2;
			pInner->pub.total <<= 2;
			pInner->p32[pInner->pub.total] = *(CRUINT32*)data;
			pInner->pub.total += 4;
			break;
		case DYN_MODE_64:
			pInner->pub.total >>= 3;
			pInner->pub.total <<= 3;
			pInner->p64[pInner->pub.total] = *(CRUINT64*)data;
			pInner->pub.total += 8;
			break;
		default:
			break;
		}
	}
    return CRTRUE;
}

static CRBOOL _inner_dyn_pop_(CRDYNAMIC dyn, void* data, CRENUM mode)
{
    PCRDYN pInner = dyn;
	if (pInner->pub.total == 0)
		return CRFALSE;
	//尚有元素可取的情况
	switch (mode)
	{
	case DYN_MODE_8:
		pInner->pub.total--;
		if (data) *(CRUINT8*)data = pInner->p8[pInner->pub.total];
		break;
	case DYN_MODE_16:
		if (pInner->pub.total % 2)
			pInner->pub.total = (pInner->pub.total >> 1) << 1;
		else pInner->pub.total -= 2;
		if (data) *(CRUINT16*)data = pInner->p16[pInner->pub.total >> 1];
		break;
	case DYN_MODE_32:
		if (pInner->pub.total % 4)
			pInner->pub.total = (pInner->pub.total >> 2) << 2;
		else pInner->pub.total -= 4;
		if (data) *(CRUINT16*)data = pInner->p32[pInner->pub.total >> 2];
		break;
	case DYN_MODE_64:
		if (pInner->pub.total % 8)
			pInner->pub.total = (pInner->pub.total >> 3) << 3;
		else pInner->pub.total -= 8;
		if (data) *(CRUINT16*)data = pInner->p64[pInner->pub.total >> 3];
		break;
	default:
		break;
	}
	if (pInner->pub.total < pInner->capacity >> 1 && pInner->capacity > 32)//可以释放一些空间
	{
		pInner->capacity >>= 1;
		pInner->arr = _inner_dyn_sizedown_(pInner->arr, pInner->capacity);
	}
	return CRTRUE;
}

CRAPI CRBOOL CRDynPush(CRDYNAMIC dyn, void* data, CRDynEnum mode)
{
    PCRDYN pInner = dyn;
    if (!pInner)
        return CRFALSE;
    CRBOOL back;
    EnterCriticalSection(&(pInner->pub.cs));
    _inner_dyn_push_(dyn, data, mode);
    LeaveCriticalSection(&(pInner->pub.cs));
    return back;
}

CRAPI CRBOOL CRDynPop(CRDYNAMIC dyn, void* data, CRDynEnum mode)
{
    PCRDYN pInner = dyn;
    if (!pInner)
        return CRFALSE;
    CRBOOL back;
    EnterCriticalSection(&(pInner->pub.cs));
    back = _inner_dyn_pop_(dyn, data, mode);
    LeaveCriticalSection(&(pInner->pub.cs));
    return back;
}

CRAPI CRBOOL CRDynSet(CRDYNAMIC dyn, void *data, CRUINT64 sub, CRDynEnum mode)
{
	PCRDYN pInner = dyn;
	if (!pInner)
		return CRFALSE;
	EnterCriticalSection(&(pInner->pub.cs));
	switch (mode)
	{
	case DYN_MODE_8:
		if (sub < pInner->pub.total)
			pInner->p8[sub] = *(CRUINT8*)data;
		else goto Push;
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	case DYN_MODE_16:
		if (sub * 2 < pInner->pub.total)
			pInner->p16[sub] = *(CRUINT16*)data;
		else goto Push;
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	case DYN_MODE_32:
		if (sub * 4 < pInner->pub.total)
			pInner->p32[sub] = *(CRUINT32*)data;
		else goto Push;
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	case DYN_MODE_64:
		if (sub * 8 < pInner->pub.total)
			pInner->p64[sub] = *(CRUINT64*)data;
		else goto Push;
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	default:
		break;
	}
Push:
	//如果超过容量了，就忽略下标，在末尾处插入扩容
	CRBOOL back = _inner_dyn_push_(dyn, data, mode);
	LeaveCriticalSection(&(pInner->pub.cs));
	return back;
}

CRAPI CRBOOL CRDynSeek(CRDYNAMIC dyn, void* data, CRUINT64 sub, CRDynEnum mode)
{
	PCRDYN pInner = dyn;
	if (pInner)
	{
		if (!data)
			return CRFALSE;
		EnterCriticalSection(&(pInner->pub.cs));
		switch (mode)
		{
		case DYN_MODE_8:
			if (sub < pInner->pub.total)
				*(CRUINT8*)data = pInner->p8[sub];
			else
				*(CRUINT8*)data = 0;
			break;
		case DYN_MODE_16:
			if (sub * 2 < pInner->pub.total)
				*(CRUINT16*)data = pInner->p16[sub];
			else
				*(CRUINT16*)data = 0;
			break;
		case DYN_MODE_32:
			if (sub * 4 < pInner->pub.total)
				*(CRUINT32*)data = pInner->p32[sub];
			else
				*(CRUINT32*)data = 0;
			break;
		case DYN_MODE_64:
			if (sub * 8 < pInner->pub.total)
				*(CRUINT64*)data = pInner->p64[sub];
			else
				*(CRUINT64*)data = 0;
			break;
		default:
			break;
		}
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	}
	return CRTRUE;
}
