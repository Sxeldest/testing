#pragma once
#include "../main.h"
#include "../util/patch.h"
#include <mutex>

class CFPSFix 
{
public:
    char unk[16];
public:
    CFPSFix(/* args */)
    {
        CPatch::CallFunction<void>(g_libSTUFF + 0x1A738 + 1, this);
    }
    ~CFPSFix()
    {
        
    }

    void PushThread(pid_t tid)
    {
        CPatch::CallFunction<void>(g_libSTUFF + 0x1AA48 + 1, this, tid);
    }

    
};
