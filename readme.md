﻿<!--
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-05-31 23:41:57
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-01 23:34:17
 * @FilePath: \Crystal-Core\readme.md
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
-->
# 晶体引擎内核
## 包含
__此内核包含一个模块加载器，一个内存管理系统，以及一个日志系统__
### -模块加载器
&emsp;模块加载器是引擎最核心的部分，此加载器用于统一动态库加载的规范，确保在各个平台尤其是Windows和Linux上使用动态库有相似的体验（不至于被动态库默认搜索路径不同而折腾到掉头发）。  
&emsp;使用加载器可以在任意时刻对模块进行热载和热分离，具体实现取决于模块本身。这里所说的模块本质上就是二进制动态库，而模块里面既可以有可执行程序，也可以有数据等，如此拆分是为了降低工程之间的耦合度（你也不喜欢被屎山代码搞得对项目无从下手吧（^^）。此外，模块化的组成也有利于引擎的更新，追随时代的脚步有时候是要付出代价的。
### -内存管理系统
&emsp;由于晶体引擎自诞生起就是多模块分散动态库设计的，所以说涉及到不同动态库之间内存申请和释放的冲突会很头疼。所以说干脆由一个最核心的动态库——内核，来全权负责内存。  
&emsp;所有内存申请释放和大小调整都只能由内核来操作。
### -日志系统
&emsp;如果想要减少掉头发，就乖乖地在关键位置接入日志（乐）。  
&emsp;由于Windows和Linux平台的debugstring有些许差别，所以说面向调试器的debugstring完全放弃，日志输出目标可以选择控制台打印，也可以选择输出到文件。
## 依赖
&emsp;引擎核心是一切伊始，没有特别的依赖项，反而是设计为被依赖的一方。  
&emsp;其他的依赖项只有C标准库了，大部分情况下标准库不会纳入讨论范畴。
## 构建
