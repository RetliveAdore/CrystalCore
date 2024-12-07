<!--
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-05-31 23:41:57
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-07 18:04:02
 * @FilePath: \CrystalCore\readme.md
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
-->
# 晶体引擎内核
## 包含
__此内核包含一个模块加载器，一个日志系统，以及一个内存管理系统__
## 构建
&emsp;此项目被设计为跨平台使用，目前可在Windows和Linux平台构建，确保已经配置好了mingw（Windows平台）或者gnu（Linux平台）环境，然后执行对应的buildall.xx脚本即可。构建编译结果以及部分中间文件可以在新生成的 out 文件夹内找到。
## 快速入门
### -模块加载器
&emsp;模块加载器是引擎最核心的部分，此加载器用于统一动态库加载的规范，确保在各个平台尤其是Windows和Linux上使用动态库有相似的体验（不至于被动态库默认搜索路径不同而折腾到掉头发）。  
&emsp;使用加载器可以在任意时刻对模块进行热载和热分离，具体实现取决于模块本身。这里所说的模块本质上就是二进制动态库，而模块里面既可以有可执行程序，也可以有数据等，如此拆分是为了降低工程之间的耦合度（你也不喜欢被屎山代码搞得对项目无从下手吧（^^）。此外，模块化的组成也有利于引擎的更新，追随时代的脚步有时候是要付出代价的。  
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
errcode = CRMemSetup(1024);
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
***
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