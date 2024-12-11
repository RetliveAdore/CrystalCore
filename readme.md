<!--
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-05-31 23:41:57
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-11 14:09:17
 * @FilePath: \CrystalCore\readme.md
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
-->
# 晶体引擎内核
## 包含
__此内核包含：模块加载器、日志系统、内存管理系统、多线程封装__
## 构造
&emsp;此项目被设计为跨平台使用，目前可在Windows和Linux平台构建，确保已经配置好了mingw（Windows平台）或者gnu（Linux平台）环境，然后执行对应的buildall.xx脚本即可。构建编译结果以及部分中间文件可以在新生成的 out 文件夹内找到。  
&emsp;通常模块编译的输出文件分为两部分，libXX.a和libXX.so，其中.a文件是内部功能清单，是非必需的。如果你能记得所有函数的接口定义，也可以按照如下方式来编写一个清单，并将其传入CRImport(...)进行加载：
~~~C
void _cr_inner_do_nothing_(void){};

void* CRCoreFunListArr[] =
{
    _cr_inner_do_nothing_, "CRLogDate",     //0
    _cr_inner_do_nothing_, "CRPrint",       //2
    //...
    0  //所有清单都必须有一个0作为结尾标识
}
void** CRCoreFunList = CRCoreFunListArr;
~~~
&emsp;如果你想要编写自己的模块，请以上述代码为标准制作接口清单。此清单通常被编译到.a文件中打包进可执行程序，以减轻用户手写清单的繁琐步骤压力。  
&emsp;而同名的.so文件时不可或缺的，其包含的所有功能的具体实现，以及数据等，如果用户愿意，也可以绕过CRImport手动加载，但过程繁琐，不推荐。
## 快速入门
### 错误处理
&emsp;晶体引擎定义了错误代码类型CRCODE，此为32为整数。每个可能会出错的函数都会返回错误代码（部分结构体创建函数除外），错误代码0为正常，其余错误类型可在宏定义注释中，或在文档中查询到，属于自定义数值。  
&emsp;对于错误的逐级传递，每一层都必须对更深层的，所有可能的错误代码做出处理，或者传递到更浅一层。最终所有的错误都将被处理，在第三方模块中，不建议出现任何错误代码丢弃，或者不返回错误代码的现象（非强制）。  
&emsp;函数功能以及错误代码的两种风格注释示例如下（非强制）：
~~~C
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
~~~
### -模块加载器
&emsp;模块加载器是引擎最核心的部分，此加载器用于统一动态库加载的规范，确保在各个平台尤其是Windows和Linux上使用动态库有相似的体验（不至于被动态库默认搜索路径不同而折腾到掉头发）。  
&emsp;使用加载器可以在任意时刻对模块进行热载和热分离，具体实现取决于模块本身。这里所说的模块本质上就是二进制动态库，而模块里面既可以有可执行程序，也可以有数据等，如此拆分是为了降低工程之间的耦合度（你也不喜欢被屎山代码搞得对项目无从下手吧（^^））。此外，模块化的组成也有利于引擎的更新，追随时代的脚步有时候是要付出代价的。  
&emsp;内核组件同样需要经过加载器加载之后才能使用，使用以下示例代码可以完成内核的模块加载：
~~~C
#include <CrystalCore.h>
#include <stdio.h>

static CRMODULE core = NULL;

int main(int argc, char** argv)
{
    //CrystalCore.so是模块文件，此处放在与可执行文件相同的路径下。
    core = CRImport("CrystalCore.so", CRCoreFunList, argv[0]);
    if (!core) printf("failed to load core\n");

    /*
    * 其他代码
    */

    //退出之前需要卸载模块
    CRUnload(core);
    return 0;
}
~~~
### -日志系统
&emsp;如果想要减少掉头发，就乖乖地在关键位置接入日志（乐）。  
&emsp;由于Windows和Linux平台的debugstring有些许差别，所以说面向调试器的debugstring完全放弃，日志输出目标可以选择控制台打印，也可以选择输出到文件。
### -内存管理系统
&emsp;由于晶体引擎自诞生起就是多模块分散动态库设计的，所以说涉及到不同动态库之间内存申请和释放的冲突会很头疼。所以说干脆由一个最核心的动态库——内核，来全权负责内存。  
&emsp;所有内存申请释放和大小调整都只能由内核来操作。内核需要初始化动态内存堆，容量由用户决定，在不初始化内存堆的情况下任何内存申请操作都无法进行。使用如下代码来初始化内存堆：
~~~C
/*
* ...
* 在内核模块加载成功的前提下
*/

CRCODE errcode = 0;
errcode = CRMemSetup(2048);
if(errcode) CR_LOG_ERR("console", "err setup");

//使用此函数查看内存池的结构状态
CRMemIterator();

/*
* ...
*/

//在结束程序前需要释放掉内存堆，通常操作系统会自动回收，但此处仍然手动演示
errcode = CRMemClear();
if(errcode) CR_LOG_ERR("console", "memory using");
~~~
&emsp;需要注意的是，动态内存堆在用户态实现，无法使用硬件的保护模式，故禁用了原生的下标访问模式，并实现了一个动态数组系统和相关接口用以申请和访问动态内存堆资源。  
&emsp;动态数组的使用示例如下：
~~~C
/*
* ...
* 在在动态内存堆创建成功的前提下
*/

CRDYNAMIC dyn1 = CRDyn(10);
if (!dyn1)
{
    CR_LOG_ERR("console", "failed to create dynamic array!");
    return 1;
}
CRMemIterator();
//动态数组使用完毕之后需要手动释放
CRFreeDyn(dyn1);
CRMemIterator();
~~~
&emsp;当然，除了动态数组，也有与其同级的数据结构如键值树、双向链表等，但只有动态数组能够给用户提供连续的，近似原生数组的内存空间。
### -多线程封装
&emsp;为了抹平Windows和Linux中多线程接口的小小差异，此处增加了一层封装。使用以下宏定义来进行多线程功能的初始化：
~~~C
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
~~~
&emsp;更多功能函数可在CrystalCore.h文件中找到。
## 完整示例代码
~~~C
/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-01 23:09:04
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-07 18:01:59
 * @FilePath: \CrystalCore\sandbox\main.c
 * @Description: 
 * 用于测试的主程序 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */

#include <CrystalCore.h>
#include <stdio.h>

typedef void*(*CRALLOC)(void* ptr, CRUINT64 size);

static CRMODULE core = NULL;

int main(int argc, char** argv)
{
    core = CRImport("CrystalCore.so", CRCoreFunList, argv[0]);
    if (!core) printf("failed to load core\n");
    CRCODE errcode = 0;
    errcode = CRMemSetup(2048);
    if(errcode) CR_LOG_ERR("console", "err setup");
    CRMemIterator();
    //
    CRDYNAMIC dyn1 = CRDyn(10);
    if (!dyn1)
    {
        CR_LOG_ERR("console", "failed to create dynamic array!");
        return 1;
    }
    CRMemIterator();
    //
    CRFreeDyn(dyn1);
    CRMemIterator();
    //
    errcode = CRMemClear();
    if(errcode) CR_LOG_ERR("console", "err clear");
    CR_LOG_IFO("console", "end");
    CRUnload(core);
    return 0;
}
~~~