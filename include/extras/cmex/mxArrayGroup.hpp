#pragma once

#include <mex.h>
#include <stdexcept>

namespace extras{namespace cmex{

    /// group of thread-safe mxArray pointers
    /// useful for capturing all of the arguments passed to a mex function and storing them in a list or queue
    class mxArrayGroup{
        size_t nArrays=0;
        mxArray ** pArray=nullptr;
    public:

        /// concetenate with another array group
        mxArrayGroup& cat(const mxArrayGroup& other){
            size_t nA2 = nArrays + other.nArrays;
            mxArray ** p2 = new mxArray* [nA2];
            for(size_t n=0;n<nArrays;++n){
                p2[n] = pArray[n];
            }
            for(size_t n=0;n<other.nArrays;++n){
                p2[n+nArrays] = other.pArray[n];
            }
            delete[] pArray;
            pArray = p2;
            nArrays = nA2;

            return *this;
        }


        /// create arrag group with nA elements
        /// all mxArray elements will be nullptr
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

        /// set a specific element of the array group
        /// NOTE: the previous array at element n will be destroyed
        /// *A will also be converted to a persistent array
        /// *A should not be deleted by any other functions after associating with
        /// the array group
        void setArray(size_t n, mxArray* A){
            if(n>=nArrays){
                throw(std::runtime_error("index exceeds ArrayGroup size"));
            }
            mexMakeArrayPersistent(A);
            mxDestroyArray(pArray[n]);
            pArray[n] = A;
        }

        /// number of arrays in the group
        size_t size() const{
            return nArrays;
        }

        /// return array of mxArray*
        operator mxArray**() {return pArray;}

        /// return array of const mxArray*
        operator const mxArray**() const {return const_cast<const mxArray**>(pArray);}        

        /// get array at index n
        const mxArray* getArray(size_t n) const{
            if(n>=nArrays){
                throw(std::runtime_error("index exceeds ArrayGroup size"));
            }
            return pArray[n];
        }

        /// copy the array group to an standard array of mxArray pointers
        /// use this to pass arrays out of a mex function
        /// NOTE: array group elements are copied, so plhs[] will not be persistent
        void copyTo(size_t nlhs, mxArray** plhs) const{
            if(nlhs>nArrays){
                throw(std::runtime_error("number of outputs exceeds size of mxArrayGroup"));
            }
            for(size_t n=0;n<nlhs;++n){
                plhs[n] = mxDuplicateArray(pArray[n]);
            }
        }

        // create array group from standard array of mxArray pointers
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

        /// Copy by value
        mxArrayGroup(const mxArrayGroup& B){
            nArrays = B.nArrays;
            pArray = new mxArray* [nArrays];
            for(size_t n=0;n<nArrays;++n){
                pArray[n] = mxDuplicateArray(B.pArray[n]);
                mexMakeArrayPersistent(pArray[n]);
            }
        }
        mxArrayGroup& operator=(const mxArrayGroup& B){
            for(size_t n=0;n<nArrays;++n){
                mxDestroyArray(pArray[n]);
            }
            if(pArray!=nullptr)
            {
                delete[] pArray;
            }

            nArrays = B.nArrays;
            pArray = new mxArray* [nArrays];
            for(size_t n=0;n<nArrays;++n){
                pArray[n] = mxDuplicateArray(B.pArray[n]);
                mexMakeArrayPersistent(pArray[n]);
            }
            return *this;
        }

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

    /// concatenate two arrays
    mxArrayGroup cat(mxArrayGroup A, const mxArrayGroup& B){
        A.cat(B);
        return A;
    }

}}
