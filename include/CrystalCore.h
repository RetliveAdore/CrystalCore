/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:54:35
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-07 16:00:25
 * @FilePath: \CrystalCore\include\CrystalCore.h
 * @Description: 这个就是核心文件头了，内部包含一个自动加载器和手动加载器
 * 自动加载器是用于加载CrystalCore.so的，手动加载器是用于加载出核心以外的所有模块的
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTALCORE_H_
#define _INCLUDE_CRYSTALCORE_H_

#include <definitions.h>
#include <CrystalLog.h>
#include <CrystalMemory.h>

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

/**
 * 创建动态数组
 * 传入：初始大小（字节）
 * 返回值：动态数组
 * 假如创建失败，返回NULL，只有初始化动态内存堆之后且容量充足才能成功创建。
 * 内部结构隐藏，用于替代原生下标访问。
 * 由于动态内存堆无法原生进行数组访问越界检测，故不开放原生下标访问方式。
 */
typedef CRDYNAMIC(*PCRDYN)(CRUINT64 size);
#define CRDyn ((PCRDYN)CRCoreFunList[18])
/**
 * 释放创建的动态数组
 */
typedef CRBOOL(*CRFREEDYN)(CRDYNAMIC dyn);
#define CRFreeDyn ((CRFREEDYN)CRCoreFunList[20])
/**
 * 获取动态数组大小
 * 返回值：字节数
 */
typedef CRUINT64(*CRDYNSIZE)(CRDYNAMIC dyn);
#define CRDynSize ((CRDYNSIZE)CRCoreFunList[22]);
/**
 * 在动态数组末尾压入数据
 * 参数1：要操作的动态数组
 * 参数2：要压入的数据
 * 参数3：字长模式
 * 返回值：压入失败为CRFALSE，否则为CRTRUE
 * 此操作会自动增加动态数组的大小，当容量增加失败时，会压入失败；
 * 字长模式可选，最小单位为字节（8比特），可选为：
 * DYN_MODE_XX，支持的字长为：8、16、32、64。
 */
typedef CRBOOL(*CRDYNPUSH)(CRDYNAMIC dyn, void* data, CRDynEnum mode);
#define CRDynPush ((CRDYNPUSH)CRCoreFunList[24])
/**
 * 从动态数组末尾弹出数据
 * 参数1：要操作的动态数组
 * 参数2：用于接收数据的指针
 * 参数3：字长模式
 * 返回值：弹出失败为CRFALSE，否则为CRTRUE
 * 此操作会自动缩小动态数组的大小，字长模式选择同CRDynPush。
 */
typedef CRBOOL(*CRDYNPOP)(CRDYNAMIC dyn, void* data, CRDynEnum mode);
#define CRDynPop ((CRDYNPOP)CRCoreFunList[26])
/**
 * 设置动态数组中某一下标的值
 * 参数1：要操作的动态数组
 * 参数2：要设置的源数据
 * 参数3：下标
 * 参数4：字长模式
 * 返回值：写入失败为CRFALSE，否则为CRTRUE
 * 下标和字长模式将共同影响实际写入的位置，位置为：下标 * 字长；
 * 当写入位置实际超过当前动态数组大小时，将自动切换到执行CRDynPush函数。
 */
typedef CRBOOL(*CRDYNSET)(CRDYNAMIC dyn, void *data, CRUINT64 sub, CRDynEnum mode);
#define CRDynSet ((CRDYNSET)CRCoreFunList[28])
/**
 * 搜寻动态数组中某一下标的值
 * 参数1：要操作的动态数组
 * 参数2：用于接收数据的指针
 * 参数3：下标
 * 参数4：字长模式
 * 返回值：无效的动态数组为CRFALSE，否则为CRTRUE
 * 寻址模式同CRDynSet，当超出时，返回0数据。
 */
typedef CRBOOL(*CRDYNSEEK)(CRDYNAMIC dyn, void* data, CRUINT64 sub, CRDynEnum mode);
#define CRDynSeek ((CRDYNSEEK)CRCoreFunList[30])

#endif
