/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-05-29 18:59:49
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-05 12:35:01
 * @FilePath: \CrystalCore\include\definitions.h
 * @Description: 包含Crystal会用到的所有共有的类型和宏定义
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CR_DEFINITIONS_H_
#define _INCLUDE_CR_DEFINITIONS_H_

#include <stdint.h>

typedef uint8_t CRUINT8;
typedef uint16_t CRUINT16;
typedef uint32_t CRUINT32;
typedef uint64_t CRUINT64;

typedef int8_t CRINT8;
typedef int16_t CRINT16;
typedef int32_t CRINT32;
typedef int64_t CRINT64;

typedef char CRCHAR;

typedef void* CRLVOID;

typedef CRUINT32 CRCODE;
typedef CRUINT32 CRENUM;
typedef CRLVOID CRMODULE;
typedef CRLVOID CRSTRUCTURE;

//

typedef double CRTIMER;
typedef CRLVOID CRTHREAD;
typedef CRLVOID CRLOCK;
typedef CRUINT64 CRWINDOW;
typedef CRLVOID CRINET;

typedef void (*CRThreadFunc)(CRLVOID user, CRTHREAD id);

#ifdef _WIN32
#  define CR_WINDOWS
#elif __linux__
#  define CR_LINUX
#else
#  error unsupported platform
#endif

#ifdef __cplusplus
#  define CRBOOL bool
#  define CRRUE true
#  define CRFALSE false
#  ifndef NULL
#    define NULL nullptr
#  endif
#else
#  define CRBOOL _Bool
#  define CRTRUE  1
#  define CRFALSE 0
#  ifndef NULL
#    define NULL (void*)0
#  endif
#endif

#ifdef CR_WINDOWS
#  define APIETY __stdcall
#elif defined CR_LINUX
#  define APIETY
#endif
#define APIETYP APIETY *

#define CRAPI __attribute__((visibility("default")))
#define CRHID __attribute__((visibility("hidden")))

#endif