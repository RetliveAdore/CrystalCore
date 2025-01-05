/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:54:35
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-05 17:49:22
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
#include <CrystalThread.h>

//什么都不做的占位函数
void _cr_inner_do_nothing_(void);

#ifndef _CRCOREFUNLIST_
#define _CRCOREFUNLIST_
/**
 * 模块函数清单传递入CRLoadMod以自动完成模块的加载
*/
extern void** CRCoreFunList;
typedef CRCODE(*CRMODINIT)(void** list);
typedef CRCODE(*CRMODUNINIT)(void);

#endif

/**
 * 加载Crystal的模块
 * 错误代码：
 * 0：正常；
 * 1：线程树创建失败；
 * 2：ID回收池创建失败。
 * 其中argv的作用是提供一个工作区路径，使用这个路径可以稳定地组合出相对于可执行文件的相对路径
 * argv可以传入NULL，但通常是
 * int main(int argc, char* argv[])
 * 中的argv[0]
*/
CRMODULE CRImport(const CRCHAR* name, void* list[], const CRCHAR* argv);
/**
 * 卸载Crystal的模块
 * 返回值：
 * 0：正常；
 * 1：线程树销毁失败；
 * 2：ID回收池销毁失败。
*/
CRCODE CRUnload(CRMODULE mod);

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
 * 返回值：
 * 0：正常；
 * 1：无效的动态数组。
 */
typedef CRCODE(*CRFREEDYN)(CRDYNAMIC dyn);
#define CRFreeDyn ((CRFREEDYN)CRCoreFunList[20])
/**
 * 获取动态数组大小
 * 传入：有效的动态数组
 * 返回值：字节数
 */
typedef CRUINT64(*CRDYNSIZE)(CRDYNAMIC dyn);
#define CRDynSize ((CRDYNSIZE)CRCoreFunList[22])
/**
 * 在动态数组末尾压入数据
 * 参数1：要操作的动态数组
 * 参数2：要压入的数据（指向数据的指针）
 * 参数3：字长模式
 * 返回值：
 * 0：正常；
 * 1：扩容失败。
 * 此操作会自动增加动态数组的大小，当容量增加失败时，会压入失败；
 * 字长模式可选，最小单位为字节（8比特），可选为：
 * DYN_MODE_XX，支持的字长为：8、16、32、64。
 */
typedef CRCODE(*CRDYNPUSH)(CRDYNAMIC dyn, void* data, CRDynEnum mode);
#define CRDynPush ((CRDYNPUSH)CRCoreFunList[24])
/**
 * 从动态数组末尾弹出数据
 * 参数1：要操作的动态数组
 * 参数2：用于接收数据的指针
 * 参数3：字长模式
 * 返回值：
 * 0：正常；
 * 1：数组为空。
 * 此操作会自动缩小动态数组的大小，字长模式选择同CRDynPush。
 */
typedef CRCODE(*CRDYNPOP)(CRDYNAMIC dyn, void* data, CRDynEnum mode);
#define CRDynPop ((CRDYNPOP)CRCoreFunList[26])
/**
 * 设置动态数组中某一下标的值
 * 参数1：要操作的动态数组
 * 参数2：要设置的源数据（指向源数据的指针）
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

/**
 * 创建一个键值树
 * 返回值：键值树
 * 假如创建失败，返回NULL，只有初始化动态内存堆之后且容量充足才能成功创建。
 */
typedef CRRBTREE(*PCRTREE)(void);
#define CRTree ((PCRTREE)CRCoreFunList[32])
/**
 * 释放创建的键值树
 * 返回值：
 * 0：正常；
 * 1：无效的键值树。
 */
typedef CRBOOL(*CRFREETREE)(CRRBTREE tree);
#define CRFreeTree ((CRFREETREE)CRCoreFunList[34])
/**
 * 获取键值树节点数量
 * 传入：有效的键值树
 * 返回值：数量
 */
typedef CRUINT64(*CRTREECOUNT)(CRRBTREE tree);
#define CRTreeCount ((CRTREECOUNT)CRCoreFunList[36])
/**
 * 往键值树中插入节点
 * 参数1：键值树
 * 参数2：要插入的数据（指针）
 * 参数3：键值
 * 返回值：
 * 0：正常；
 * 1：无效的键值树；
 * 2：内存申请失败。
 */
typedef CRCODE(*CRTREEPUT)(CRRBTREE tree, CRLVOID data, CRINT64 key);
#define CRTreePut ((CRTREEPUT)CRCoreFunList[38])
/**
 * 从键值树中移除节点
 * 参数1：键值树
 * 参数2：用于承载结果的指针
 * 参数3：键值
 * 返回值：
 * 0：正常；
 * 1：无效的键值树；
 * 2：空树；
 * 3：未找到。
 * 如果没有找到对应键值的节点，将不进行任何操作。
 */
typedef CRCODE(*CRTREEGET)(CRRBTREE tree, CRLVOID* data, CRINT64 key);
#define CRTreeGet ((CRTREEGET)CRCoreFunList[40])
/**
 * 在键值树中搜索节点
 * 参数1：键值树
 * 参数2：用于承载结果的指针
 * 参数3：键值
 * 返回值：
 * 0：正常；
 * 1：无效的键值树；
 * 2：空树；
 * 3：未找到。
 */
typedef CRCODE(*CRTREESEEK)(CRRBTREE tree, CRLVOID* data, CRINT64 key);
#define CRTreeSeek ((CRTREESEEK)CRCoreFunList[42])

/**
 * 遍历动态数组
 * 参数1：动态数组
 * 参数2：回调函数
 * 参数3：用户数据（供回调函数使用）
 * 返回值：
 * 0：遍历成功；
 * 1：无效的动态数组；
 * 2：无效的回调函数。
 * 此遍历会以CRLVOID为单位将数组中的元素取出，然后传入回调函数执行。
 * 此操作不会对数组中数据进行修改。
 */
typedef CRCODE(*CRDYNITERATOR)(CRDYNAMIC dyn, IteratorCallback cal, CRLVOID user);
#define CRDynIterator ((CRDYNITERATOR)CRCoreFunList[44])
/**
 * 遍历键值树
 * 参数1：键值树
 * 参数2：回调函数
 * 参数3：用户数据（供回调函数使用）
 * 返回值：
 * 0：遍历成功；
 * 1：无效的键值树；
 * 2：无效的回调函数。
 * 此遍历使用中序遍历。
 * 此操作不会对节点和节点内数据有任何修改。
 */
typedef CRCODE(*CRTREEITERATOR)(CRRBTREE tree, IteratorCallback cal, CRLVOID user);
#define CRTreeIterator ((CRTREEITERATOR)CRCoreFunList[46])

/**
 * 初始化多线程功能
 * 返回值：
 * 0：正常；
 * 1：线程树创建失败；
 * 2：回收池创建失败。
 * 此功能依赖动态内存堆，请务必保证进行初始化时已经创建内存堆且容量充足。
 */
typedef CRCODE(*CRTHREADINIT)(void);
#define CRThreadInit ((CRTHREADINIT)CRCoreFunList[48])
/**
 * 逆初始化多线程功能
 * 返回值：
 * 0：正常；
 * 1：线程树销毁失败；
 * 2：回收池销毁失败。
 * 此函数将多线程功能还原到未初始化的状态，但不会终止已经创建的线程。
 */
typedef CRCODE(*CRTHREADUNINIT)(void);
#define CRThreadUninit ((CRTHREADUNINIT)CRCoreFunList[50])
/**
 * 线程休眠
 * 传入休眠时间（ms）
 * 无返回值
 */
typedef void(*CRSLEEP)(CRUINT64 ms);
#define CRSleep ((CRSLEEP)CRCoreFunList[52])
/**
 * 创建线程
 * 参数1：线程函数
 * 参数2：用户数据
 * 返回值：线程ID，其中0为非法ID。
 */
typedef CRTHREAD(*PCRTHREAD)(CRThreadFunc, CRLVOID data);
#define CRThread ((PCRTHREAD)CRCoreFunList[54])
/**
 * 等待线程
 * 参数1：线程ID
 * 无返回值。
 * 线程结束或不存在时返回。
 */
typedef void(*CRWAITTHREAD)(CRTHREAD thread);
#define CRWaitThread ((CRWAITTHREAD)CRCoreFunList[56])
/**
 * 创建锁
 */
typedef CRLOCK(*CRLOCKCREATE)(void);
#define CRLockCreate ((CRLOCKCREATE)CRCoreFunList[58])
/**
 * 释放锁
 */
typedef void(*CRLOCKRELEASE)(CRLOCK lock);
#define CRLockRelease ((CRLOCKRELEASE)CRCoreFunList[60])
/**
 * 加锁
 */
typedef void(*PCRLOCK)(CRLOCK lock);
#define CRLock ((PCRLOCK)CRCoreFunList[62])
/**
 * 解锁
 */
typedef void(*CRUNLOCK)(CRLOCK lock);
#define CRUnlock ((CRUNLOCK)CRCoreFunList[64])
/**
 * 标记计时器
 * 参数：指向计时器的指针
 * 返回值：距离上次标记的时间（秒）
 * 计时器不需要专门创建，直接声明一个即可（双精度浮点数），
 * 此函数将标记计时器，后续的计时都将以此为起点，直到再次标记。
 */
typedef double(*CRTIMERMARK)(CRTIMER *pTimer);
#define CRTimerMark ((CRTIMERMARK)CRCoreFunList[66])
/**
 * 窥探计时器
 * 参数：指向计时器的指针
 * 返回值：距离上次标记的时间（秒）
 * 计时器不需要专门创建，直接声明一个即可（双精度浮点数），
 * 此函数不会标记计时器，仅返回时间。
 */
typedef double(*CRTIMERPEEK)(CRTIMER *pTimer);
#define CRTimerPeek ((CRTIMERPEEK)CRCoreFunList[68])

#endif
