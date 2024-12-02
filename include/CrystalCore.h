/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:54:35
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-02 22:56:01
 * @FilePath: \CrystalCore\include\CrystalCore.h
 * @Description: 这个就是核心文件头了，内部包含一个自动加载器和手动加载器
 * 自动加载器是用于加载CrystalCore.so的，手动加载器是用于加载出核心以外的所有模块的
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTALCORE_H_
#define _INCLUDE_CRYSTALCORE_H_

#include <definitions.h>
#include <CrystalLog.h>

//什么都不做的占位函数
void _cr_inner_do_nothing_(void);

#ifndef _CRCOREFUNLIST_
#define _CRCOREFUNLIST_
/**
 * 模块函数清单传递入CRLoadMod以自动完成模块的加载
*/
extern void** CRCoreFunList;
typedef CRBOOL(*CRMODINIT)(void** list);
typedef void(*CRMODUNINIT)(void);

#endif

/**
 * 加载Crystal的模块
 * 其中argv的作用是提供一个工作区路径，使用这个路径可以稳定地组合出相对于可执行文件的相对路径
 * argv可以传入NULL，但通常是
 * int main(int argc, char* argv[])
 * 中的argv[0]
*/
CRMODULE CRImport(const CRCHAR* name, void* list[], const CRCHAR* argv);
/**
 * 卸载Crystal的模块
*/
void CRUnload(CRMODULE mod);

/**
 * 日志系统中用于获取时间的函数
 * 可以获取精确到毫秒的时间
*/
typedef CRDate(*CRLOGDATE)();
#define CRLogDate ((CRLOGDATE)CRCoreFunList[0])
/**
 * 用于彩色打印的函数，相比printf，增加了颜色选择
*/
typedef CRINT64(*CRPRINT)(CRTextColor color, const CRCHAR* fmt, ...);
#define CRPrint ((CRPRINT)CRCoreFunList[2])
/**
 * 实际的用于日志处理和输出的函数
 * 需要配合下面的宏来使用
*/
typedef void(*CRTRACE)(const CRCHAR* target, CRUINT8 level, const CRCHAR* file, CRUINT32 line, const CRCHAR* function, const CRCHAR* fmt, ...);
#define CRTrace ((CRTRACE)CRCoreFunList[4])
/**
 * 如果不想每次都使用默认的“时间戳”作为文件名，
 * 可以自己设置日志文件的文件名和路径
*/
typedef void(*CRSETLOGFILE)(const CRCHAR* path);
#define CRSetLogFile ((CRSETLOGFILE)CRCoreFunList[6])
/**
 * 设置默认的日志参数
*/
typedef void(*CRLOGDEFAULT)(const CRCHAR* tag, CRUINT8 level);
#define CRLogDefault ((CRLOGDEFAULT)CRCoreFunList[8])

/*
关于tag的取值，有两个有效取值："console"和"file"
"console"取值表明日志将输出到控制台直接显示出来
"file"取值表明日志将会输出到文件
*/

#define CR_LOG(...) CRTrace("console",1,__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#define CR_LOG_IFO(tag, ...) CRTrace(tag,0,__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#define CR_LOG_DBG(tag, ...) CRTrace(tag,1,__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#define CR_LOG_WAR(tag, ...) CRTrace(tag,2,__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#define CR_LOG_ERR(tag, ...) CRTrace(tag,3,__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)

//

/**
 * 用于申请内存或调整内存,
 * 使用之前需要先调用CRMemSetup进行初始化
*/
typedef void*(*CRALLOC)(void* ptr, CRUINT64 size);
#define CRAlloc ((CRALLOC)CRCoreFunList[10])
/**
 * 初始化动态内存堆
 * 传入初始化的内存字节数，
 * 初始化之后，在没有分配内存的情况下，可以重新初始化。
 * 程序结束之后需要执行CRMemClear来释放内存堆。
 * 返回值：
 * 0：初始化成功；
 * 1：内存申请失败；
 * 2：正在使用中的内存。
 * 3：传入的容量（size）过小
 */
typedef CRCODE(*CRMEMSETUP)(CRUINT64 size);
#define CRMemSetup ((CRMEMSETUP)CRCoreFunList[12])
/**
 * 释放动态内存堆
 * 此时无论是否有正在使用的内存块都将直接释放堆，请确保在退出的最后一步执行。
 * 释放之后可以使用CRMemSetup重新初始化内存堆
 * 返回值：
 * 0：正常释放；
 * 1：有正在使用的块（仍然释放）；
 * 2：尚未初始化。
 */
typedef CRCODE(*CRMEMCLEAR)(void);
#define CRMemClear ((CRMEMCLEAR)CRCoreFunList[14])
/**
 * 遍历动态内存堆
 * 将所有正在使用的内存块一一列出，仅提供显示，无其他操作。
 * 返回值：无
 */
typedef void(*CRMEMITERATOR)(void);
#define CRMemIterator ((CRMEMITERATOR)CRCoreFunList[16])

#endif
