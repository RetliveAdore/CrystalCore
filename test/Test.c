/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-05 15:01:11
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-05 19:48:58
 * @FilePath: \CrystalCore\test\Test.c
 * @Description: 
 * 用于测试的主程序
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalCore.h>
#include <stdio.h>

static CRMODULE core = NULL;

static void _inner_dyn_iterator_8_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRUINT8 *tmp = (CRUINT8*)&data;
    CRPrint(CR_TC_GREEN, "%d, %d, %d, %d, %d, %d, %d, %d, key：%d\n",
        tmp[0], tmp[1], tmp[2], tmp[3],
        tmp[4], tmp[5], tmp[6], tmp[7],
        key
    );
}

static void _inner_dyn_iterator_16_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRUINT16 *tmp = (CRUINT16*)&data;
    CRPrint(CR_TC_GREEN, "%d, %d, %d, %d, key：%d\n", tmp[0], tmp[1], tmp[2], tmp[3], key);
}

static void _inner_dyn_iterator_32_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRUINT32 *tmp = (CRUINT32*)&data;
    CRPrint(CR_TC_GREEN, "%d, %d, key：%d\n", tmp[0], tmp[1], key);
}

static void _inner_dyn_iterator_64_callback_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRPrint(CR_TC_GREEN, "%d, key：%d\n", data, key);
}

int main(int argc, char **argv)
{
    core = CRImport("CrystalCore.so", CRCoreFunList, argv[0]);
    if (!core) printf("failed to load core\n");
    //
    CRCODE err = 0;
    err = CRMemSetup(1024 * 1024);
    if(err) CR_LOG_ERR("console", "err setup");
    //
    CRPrint(CR_TC_LIGHTGREEN, "测试运行中\n");
    //
    CR_LOG_IFO("console", "动态数组测试：");
    CRDYNAMIC dyn1 = CRDyn(0);
    CRDYNAMIC dyn2 = CRDyn(0);
    CRDYNAMIC dyn3 = CRDyn(0);
    CRDYNAMIC dyn4 = CRDyn(0);

    CRUINT64 size1 = 64;
    CRUINT64 size2 = 32;
    CRUINT64 size3 = 16;
    CRUINT64 size4 = 16;
    for (CRUINT64 i = 0; i < size1; i++)
        CRDynPush(dyn1, &i, DYN_MODE_8);
    for (CRUINT64 i = 0; i < size2; i++)
        CRDynPush(dyn2, &i, DYN_MODE_16);
    for (CRUINT64 i = 0; i < size3; i++)
        CRDynPush(dyn3, &i, DYN_MODE_32);
    for (CRUINT64 i = 0; i < size4; i++)
        CRDynPush(dyn4, &i, DYN_MODE_64);
    CRPrint(CR_TC_BLUE, "8位测试：\n");
    CRPrint(CR_TC_BLUE, "动态数组当前容量：%d，预期：%d\n", CRDynSize(dyn1), size1);
    CRDynIterator(dyn1, _inner_dyn_iterator_8_callback_, NULL);
    CRPrint(CR_TC_BLUE, "16位测试：\n");
    CRPrint(CR_TC_BLUE, "动态数组当前容量：%d，预期：%d\n", CRDynSize(dyn2), size2 * 2);
    CRDynIterator(dyn2, _inner_dyn_iterator_16_callback_, NULL);
    CRPrint(CR_TC_BLUE, "32位测试：\n");
    CRPrint(CR_TC_BLUE, "动态数组当前容量：%d，预期：%d\n", CRDynSize(dyn3), size3 * 4);
    CRDynIterator(dyn3, _inner_dyn_iterator_32_callback_, NULL);
    CRPrint(CR_TC_BLUE, "64位测试：\n");
    CRPrint(CR_TC_BLUE, "动态数组当前容量：%d，预期：%d\n", CRDynSize(dyn4), size4 * 8);
    CRDynIterator(dyn4, _inner_dyn_iterator_64_callback_, NULL);

    CRFreeDyn(dyn1);
    CRFreeDyn(dyn2);
    //
    CR_LOG_IFO("console", "计时器测试：");
    CRTIMER timer1;
    double timePeriod1 = 0.0f;
    double timePeriod2 = 0.0f;
    double timePeriod3 = 0.0f;

    CRTimerMark(&timer1);
    CRSleep(1000);
    timePeriod1 = CRTimerPeek(&timer1);
    CRSleep(1000);
    timePeriod2 = CRTimerMark(&timer1);
    CRSleep(1000);
    timePeriod3 = CRTimerMark(&timer1);
    CRPrint(CR_TC_BLUE, "休眠1000ms后CRTimerPeek(pTimer)结果：%.6f秒\n", timePeriod1);
    CRPrint(CR_TC_BLUE, "继续休眠1000ms后CRTimerMark(pTimer)结果：%.6f秒\n", timePeriod2);
    CRPrint(CR_TC_BLUE, "继续休眠1000ms后CRTimerMark(pTimer)结果：%.6f秒\n", timePeriod3);
    //
    CRUnload(core);
    return 0;
}