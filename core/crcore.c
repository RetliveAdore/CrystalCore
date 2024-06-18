/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-01 23:53:49
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-18 14:46:23
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

void* CRCoreFunList[] =
{
    _cr_inner_do_nothing_, "CRLogDate",
    _cr_inner_do_nothing_, "CRPrint",
    _cr_inner_do_nothing_, "CRTrace",
    _cr_inner_do_nothing_, "CRSetLogFile",
    _cr_inner_do_nothing_, "CRLogDefault",
    //
    _cr_inner_do_nothing_, "CRAlloc",
    0 //检测到0表示清单结尾
};
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
    if (_inner_init_mod_) _inner_init_mod_(CRCoreFunList);
    //
    while(list[i])
    {
        #ifdef CR_WINDOWS
        list[i] = GetProcAddress(pInner->mod, list[i + 1]);
        #elif defined CR_LINUX
        list[i] = dlsym(pInner->mod, list[i + 1]);
        #endif
        if (!list[i]) list[i] = _cr_inner_do_nothing_;
        i += 2;
    }
    if (argv) free(*((CRCHAR**)&name));
    return pInner;
}

void CRUnload(CRMODULE mod)
{
    if (!mod) return;
    CRMODULEINNER* pInner = mod;
    CRUINT64 i = 0;
    while(pInner->list[i])
    {
        pInner->list[i] = _cr_inner_do_nothing_;
        i += 2;
    }
    #ifdef CR_WINDOWS
    _inner_uninit_mod_ = (CRMODUNINIT)GetProcAddress(pInner->mod, "CRModUninit");
    if (_inner_uninit_mod_) _inner_uninit_mod_();
    FreeLibrary(pInner->mod);
    #elif defined CR_LINUX
    _inner_uninit_mod_ = (CRMODUNINIT)dlsym(pInner->mod, "CRModUninit");
    if (_inner_uninit_mod_) _inner_uninit_mod_();
    dlclose(pInner->mod);
    #endif
    free(mod);
}