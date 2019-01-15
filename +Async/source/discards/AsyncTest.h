#pragma once

#include "AsyncMexProcessor.h"
#include <chrono>

class AsyncTest:public mexproc::AsyncMexProcessor
{
protected:
    virtual mxArrayGroup ProcessTask(const mxArrayGroup& Args){
        using namespace std::chrono_literals;
        mxArrayGroup out(Args.size());

        std::this_thread::sleep_for(2s);

        for(size_t n=0;n<out.size();++n){
            out.setArray(n,mxDuplicateArray(Args.getArray(n)));
        }
        return out;
    }
};
