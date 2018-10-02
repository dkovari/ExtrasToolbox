#pragma once

#include <mex.h>
#include <stdexcept>

//collection of thread-safe mxArray pointers
class mxArrayGroup{
    size_t nArrays=0;
    mxArray ** pArray=nullptr;
public:

    mxArrayGroup(size_t nA){
        nArrays = nA;
        if(nA==0){
            pArray=nullptr;
            return;
        }
        pArray = new mxArray* [nA];
        for(size_t n=0;n<nA;++n){
            pArray[n]=nullptr;
        }
    }

    void setArray(size_t n, mxArray* A){
        if(n>=nArrays){
            throw(std::runtime_error("index exceeds ArrayGroup size"));
        }
        mexMakeArrayPersistent(A);
        mxDestroyArray(pArray[n]);
        pArray[n] = A;
    }

    size_t size() const{
        return nArrays;
    }

    const mxArray* getArray(size_t n) const{
        if(n>=nArrays){
            throw(std::runtime_error("index exceeds ArrayGroup size"));
        }
        return pArray[n];
    }

    void copyTo(size_t nlhs, mxArray** plhs) const{
        if(nlhs>nArrays){
            throw(std::runtime_error("number of outputs exceeds size of mxArrayGroup"));
        }
        for(size_t n=0;n<nlhs;++n){
            plhs[n] = mxDuplicateArray(pArray[n]);
        }
    }

    mxArrayGroup(size_t nA, const mxArray** pA){
        nArrays = nA;

        if(nA==0){
            pArray=nullptr;
            return;
        }

        pArray = new mxArray* [nA];
        for(size_t n=0;n<nA;++n){
            pArray[n] = mxDuplicateArray(pA[n]);
            mexMakeArrayPersistent(pArray[n]);
        }
    }

    mxArrayGroup(mxArrayGroup&& B){
        nArrays = B.nArrays;
        pArray = B.pArray;
        B.nArrays = 0;
        B.pArray = nullptr;
    }
    mxArrayGroup& operator=(mxArrayGroup&& B){
        for(size_t n=0;n<nArrays;++n){
            mxDestroyArray(pArray[n]);
        }
        if(pArray!=nullptr)
        {
            delete[] pArray;
        }

        nArrays = B.nArrays;
        pArray = B.pArray;
        B.nArrays = 0;
        B.pArray = nullptr;
        return *this;
    }


    mxArrayGroup(const mxArrayGroup& B) = delete; //don't allow copy by value
    mxArrayGroup& operator=(const mxArrayGroup& B) = delete;//don't allow copy by value

    ~mxArrayGroup(){
        for(size_t n=0;n<nArrays;++n){
            mxDestroyArray(pArray[n]);
        }
        if(pArray!=nullptr)
        {
            delete[] pArray;
        }
    }
};
