/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:53:49
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-11 16:13:50
 * @FilePath: \CrystalCore\core\crcore.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalCore.h>
#include <malloc.h>
#include <stdio.h>

/*
* CrystalMemory
*/

#ifdef CR_WINDOWS
#include <libloaderapi.h>
#elif defined CR_LINUX
#include <dlfcn.h>
#endif

typedef struct CRMODULEINNER
{
    void** list;
    #ifdef CR_WINDOWS
    HMODULE mod;
    #elif defined CR_LINUX
    void* mod;
    #endif
}CRMODULEINNER;

static CRCHAR Path[256] = {'\0'};

void _cr_inner_do_nothing_(void){};

void* CRCoreFunListArr[] =
{
    _cr_inner_do_nothing_, "CRLogDate",     //0
    _cr_inner_do_nothing_, "CRPrint",       //2
    _cr_inner_do_nothing_, "CRTrace",       //4
    _cr_inner_do_nothing_, "CRSetLogFile",  //6
    _cr_inner_do_nothing_, "CRLogDefault",  //8
    //
    _cr_inner_do_nothing_, "CRAlloc",       //10
    _cr_inner_do_nothing_, "CRMemSetup",    //12
    _cr_inner_do_nothing_, "CRMemClear",    //14
    _cr_inner_do_nothing_, "CRMemIterator", //16
    //
    _cr_inner_do_nothing_, "CRDyn",         //18
    _cr_inner_do_nothing_, "CRFreeDyn",     //20
    _cr_inner_do_nothing_, "CRDynSize",     //22
    _cr_inner_do_nothing_, "CRDynPush",     //24
    _cr_inner_do_nothing_, "CRDynPop",      //26
    _cr_inner_do_nothing_, "CRDynSet",      //28
    _cr_inner_do_nothing_, "CRDynSeek",     //30
    //
    _cr_inner_do_nothing_, "CRTree",        //32
    _cr_inner_do_nothing_, "CRFreeTree",    //34
    _cr_inner_do_nothing_, "CRTreeCount",   //36
    _cr_inner_do_nothing_, "CRTreePut",     //38
    _cr_inner_do_nothing_, "CRTreeGet",     //40
    _cr_inner_do_nothing_, "CRTreeSeek",    //42
    //
    _cr_inner_do_nothing_, "CRDynIterator", //44
    _cr_inner_do_nothing_, "CRTreeIterator",//46
    //
    _cr_inner_do_nothing_, "CRThreadInit",  //48
    _cr_inner_do_nothing_, "CRThreadUninit",//50
    _cr_inner_do_nothing_, "CRSleep",       //52
    _cr_inner_do_nothing_, "CRThread",      //54
    _cr_inner_do_nothing_, "CRWaitThread",  //56
    _cr_inner_do_nothing_, "CRLockCreate",  //58
    _cr_inner_do_nothing_, "CRLockRelease", //60
    _cr_inner_do_nothing_, "CRLock",        //62
    _cr_inner_do_nothing_, "CRUnlock",      //64
    //
    0 //检测到0表示清单结尾
};
void** CRCoreFunList = CRCoreFunListArr;
static CRMODINIT _inner_init_mod_ = NULL;
static CRMODUNINIT _inner_uninit_mod_ = NULL;

static const CRCHAR* _inner_conv_path_(const CRCHAR* name, const CRCHAR* argv)
{
    CRCHAR* back;
    CRUINT64 l1 = 0;
    CRUINT64 l2 = 0;
    CRUINT64 i = 0;
    while (argv[i] != '\0')
    {
        #ifdef CR_WINDOWS
        if (argv[i] == '\\') l1 = i + 1;
        #elif defined CR_LINUX
        if (argv[i] == '/') l1 = i + 1;
        #endif
        i++;
    }
    while (name[l2] != 0) l2++;   
    back = malloc(l1 + l2 + 1);
    if (!back) return name;
    for (int p = 0; p < l1; p++)
        back[p] = argv[p];
    for (int p = 0; p < l2; p++)
    {
        #ifdef CR_WINDOWS
        if (name[p] == '/') back[p + l1] = '\\';
        else back[p + l1] = name[p];
        #else
        back[p + l1] = name[p];
        #endif
    }
    back[l1 + l2] = '\0';
    return back;
}

CRMODULE CRImport(const CRCHAR* name, void* list[], const CRCHAR* argv)
{
    if (!name || !list) return NULL;
    if (argv) name = _inner_conv_path_(name, argv);
    CRMODULEINNER* pInner = malloc(sizeof(CRMODULEINNER));
    if (!pInner) return NULL;
    pInner->list = list;
    #ifdef CR_WINDOWS
    pInner->mod = LoadLibraryA(name);
    #elif defined CR_LINUX
    pInner->mod = dlopen(name, RTLD_LAZY);
    #endif 
    if (!pInner->mod)
    {
        free(pInner);
        return NULL;
    }
    CRUINT64 i = 0;
    //
    #ifdef CR_WINDOWS
    _inner_init_mod_ = (CRMODINIT)GetProcAddress(pInner->mod, "CRModInit");
    #elif defined CR_LINUX
    _inner_init_mod_ = (CRMODINIT)dlsym(pInner->mod, "CRModInit");
    #endif
    CRCODE err = 0;
    if (_inner_init_mod_) err = _inner_init_mod_(CRCoreFunList);
    //
    while(list[i])
    {
        #ifdef CR_WINDOWS
        list[i] = GetProcAddress(pInner->mod, list[i + 1]);
        #elif defined CR_LINUX
        list[i] = dlsym(pInner->mod, list[i + 1]);
        #endif
        if (!list[i])
        {
            CR_LOG_ERR("auto", "Faild load: %s", list[i + 1]);
            list[i] = _cr_inner_do_nothing_;
        }
        i += 2;
    }
    if (argv) free(*((CRCHAR**)&name));
    if (err) CR_LOG_ERR("auto", "error importing module, error code: %d", err);
    return pInner;
}

CRCODE CRUnload(CRMODULE mod)
{
    if (!mod) return 1;
    CRMODULEINNER* pInner = mod;
    CRUINT64 i = 0;
    CRCODE err = 0;
    while(pInner->list[i])
    {
        pInner->list[i] = _cr_inner_do_nothing_;
        i += 2;
    }
    #ifdef CR_WINDOWS
    _inner_uninit_mod_ = (CRMODUNINIT)GetProcAddress(pInner->mod, "CRModUninit");
    if (_inner_uninit_mod_) err = _inner_uninit_mod_();
    if (err) CR_LOG_ERR("auto", "error unloading module, error code: %d", err);
    FreeLibrary(pInner->mod);
    #elif defined CR_LINUX
    _inner_uninit_mod_ = (CRMODUNINIT)dlsym(pInner->mod, "CRModUninit");
    if (_inner_uninit_mod_) _inner_uninit_mod_();
    dlclose(pInner->mod);
    #endif
    free(mod);
    return err;
}