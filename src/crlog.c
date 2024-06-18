/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-05-29 18:26:08
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-18 15:59:02
 * @FilePath: \CrystalCore\src\crlog.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalLog.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>

#ifdef CR_WINDOWS
#include <windows.h>
#include <io.h>
#elif defined CR_LINUX
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE  //要注意这个是必需定义的，否则没有stdout
#endif
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#endif

#define BUFFER_SIZE 2048
#define MAX_STRLEN BUFFER_SIZE

//一次性函数的设计模式，用于防止重复初始化
static void _inner_init_logfile_name_(void);
static void _inner_init_null_(void) {}  //什么也不做
typedef void (*disposableFunctionInit)(void);
static disposableFunctionInit _inner_init_ = _inner_init_logfile_name_;

static CRCHAR logFileName[1024];
static CRBOOL initedLogFile = CRFALSE;
static const CRCHAR defaultTag[] = "console";
static const CRCHAR* tag = defaultTag;
static CRUINT8 defaultLevel = 0;

static const CRCHAR* slevel[] =
{
    "INFO", "DEBUG", "WARNING", "ERROR",
    0
};
#define SLEVEL_MAX (sizeof(slevel) / sizeof(CRCHAR*) - 1)

static CRUINT32 _inner_strlen_(const CRCHAR* str)
{
    CRUINT32 len = 0;
    while (str[len] != '\0' && len <= MAX_STRLEN) len++;
    return len;
}


//declaration
CRAPI void CRSetLogFile(const CRCHAR* path);
CRAPI void CRTrace(const CRCHAR* target, CRUINT8 level, const CRCHAR* file, CRUINT32 line, const CRCHAR* function, const CRCHAR* fmt, ...);
CRAPI CRINT64 CRPrint(CRTextColor color, const CRCHAR* fmt, ...);
CRAPI CRDate CRLogDate();


/**
 * 当字符串不同时返回CRFALSE否则返回CRTRUE
*/
static CRBOOL _inner_compare_string_(const CRCHAR* str1, const CRCHAR* str2)
{
    CRUINT32 len1 = _inner_strlen_(str1);
    if (len1 != _inner_strlen_(str2)) return CRFALSE;
    for (int i = 0; i < len1; i++) if (str1[i] != str2[i]) return CRFALSE;
    return CRTRUE;
}

static void _inner_init_logfile_name_(void)
{
    CRDate date = CRLogDate();
    int ret = snprintf(logFileName, sizeof(logFileName), "%d_%d_%d_%d_%d_%d.log",
        date.year, date.month, date.day, date.hour, date.min, date.sec);
    if (ret < 0) return;
    _inner_init_ = _inner_init_null_;  //修改函数指针，下次调用时将定向到什么都不做的函数
}

void CRSetLogFile(const CRCHAR* path)
{
    snprintf(logFileName, sizeof(logFileName), "%s", path);
    _inner_init_ = _inner_init_null_;
}

CRAPI void CRLogDefault(const CRCHAR* tagIn, CRUINT8 level)
{
    if (!tagIn) tag = defaultTag;
    else tag = tagIn;
    defaultLevel = level;
}

CRAPI void CRTrace(const CRCHAR* target, CRUINT8 level, const CRCHAR* file, CRUINT32 line, const CRCHAR* function, const CRCHAR* fmt, ...)
{
    if (level < defaultLevel) return;
    va_list ap;
    CRCHAR* buffer;
    CRINT32 ret;
    CRBOOL logErr = CRFALSE;
    static CRCHAR* errBuffer = "Crystal Log Error\n";
    //缓冲区大小是有限的
    if (!(buffer = malloc(BUFFER_SIZE))) return;
    ret = snprintf(buffer, BUFFER_SIZE, "%s(%d) <%s>-<%s>[%s]: ",
        file, line, slevel[level], target, function
    );
    if (ret < 0) goto Error;
    ret = _inner_strlen_(buffer);
    va_start(ap, fmt);
    ret = vsnprintf(buffer + ret, BUFFER_SIZE - ret, fmt, ap);
    va_end(ap);
    if (ret < 0) goto Error;
    goto Throw;
Error:  //如果出现错误直接抛出错误
    logErr = CRTRUE;
    buffer = errBuffer;
    level = SLEVEL_MAX;  //如果日志本身出错了，那就直接视为最高等级错误
Throw:
    //生成日志字符串和输出日志是独立的两个部分
    if (!target)
        goto nullTarget;
    if (_inner_compare_string_("auto", target))
        target = tag;
    if (_inner_compare_string_("console", target))
    {
        switch (level)
        {
        case 0:  //info
            CRPrint(CR_TC_LIGHTGRAY, buffer);
            break;
        case 1:  //debug
            CRPrint(CR_TC_LIGHTCYAN, buffer);
            break;
        case 2:  //warning
            CRPrint(CR_TC_LIGHTMAGENTA, buffer);
            break;
        case 3:  //error
            CRPrint(CR_TC_LIGHTRED, buffer);
            break;
        default:  //默认按照debug等级处理
            CRPrint(CR_TC_LIGHTBLUE, buffer);
            break;
        }
        printf("\n");
    }
    else if (_inner_compare_string_("file", target))
    {
        //初始化文件名（一次性函数，重复调用不影响）
        _inner_init_();
        FILE* fp = fopen(logFileName, "a+");
        if (fp)
        {
            fwrite(buffer, _inner_strlen_(buffer), 1, fp);
            fwrite("\n", 1, 1, fp);
            fclose(fp);
        }
    }
    else
        goto nullTarget;
    goto End;
nullTarget:  //target为else和NULL指针的处理是一样的
    #ifdef CR_WINDOWS
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
    #endif
End:
    if (!logErr) free(buffer);
}

//
//
//下方是关于控制台颜色控制的内容
//

#ifndef CR_WINDOWS  //Windows下用不到这些
static const char *ansiColorSequences[] =
{
    "\x1B[0;30m",
    "\x1B[0;34m",
    "\x1B[0;32m",
    "\x1B[0;36m",
    "\x1B[0;31m",
    "\x1B[0;35m",
    "\x1B[0;33m",
    "\x1B[0;37m",
    "\x1B[1;30m",
    "\x1B[1;34m",
    "\x1B[1;32m",
    "\x1B[1;36m",
    "\x1B[1;31m",
    "\x1B[1;35m",
    "\x1B[1;33m",
    "\x1B[1;37m"
};

static const char *ansiColorTerms[] =
{
    "xterm",
    "rxvt",
    "vt100",
    "linux",
    "screen",
    0
};

//用于判定是否为支持彩色字符的终端
static CRBOOL _inner_isansicolor_term_(void)
{
    CRCHAR* term = getenv("TERM");
    for (const char **ansiTerm = &ansiColorTerms[0]; *ansiTerm; ++ansiTerm)
    {
        int match = 1;
        const char *t = term;
        const char *a = *ansiTerm;
        while (*a && *t)
        {
            if (*a++ != *t++)
            {
                match = 0;
                break;
            }
        }
        if (match) return CRTRUE;
    }
    return CRFALSE;
}
#endif

static void _inner_set_text_color_(CRTextColor color)
{
    #ifdef CR_WINDOWS
    HANDLE out = (HANDLE)_get_osfhandle(1);  //stdin 0, stdout 1, stderr 2
    DWORD outType = GetFileType(out);
    DWORD mode;
    if (outType == FILE_TYPE_CHAR && GetConsoleMode(out, &mode))
        SetConsoleTextAttribute(out, color);
    #else
    int outfd = fileno(stdout);  //stdin 0, stdout 1, stderr 2
    if (isatty(outfd) && _inner_isansicolor_term_())
        fputs(ansiColorSequences[color], stdout);
    #endif
}

CRAPI CRINT64 CRPrint(CRTextColor color, const CRCHAR* fmt, ...)
{
    va_list ap;
    _inner_set_text_color_(color);
    va_start(ap, fmt);
    CRUINT64 back = vprintf(fmt, ap);  //注意，不定长参数再传递就需要用到支持va_list的函数了
    va_end(ap);
    _inner_set_text_color_(CR_TC_LIGHTGRAY);
    return back;
}

CRAPI CRDate CRLogDate()
{
    CRDate date;
    #ifdef CR_WINDOWS
    GetLocalTime((LPSYSTEMTIME)&date);
    #elif defined CR_LINUX
    static time_t t = 0;
    static struct tm* ptm = NULL;
    static struct timeval ti = {0};
    static struct timespec ts = {0};
    t = time(0);
    ptm = localtime(&t);
    date.year = ptm->tm_year + 1900; //在Linux就只能手动搬砖了
	date.month = ptm->tm_mon + 1;
	date.day = ptm->tm_mday;
	date.dweek = ptm->tm_wday;
	date.hour = ptm->tm_hour;
	date.min = ptm->tm_min;
	date.sec = ptm->tm_sec;
	gettimeofday(&ti, NULL);
    date.ms = ti.tv_usec % 1000000 / 1000;
    #endif
    return date;
}