/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <mex.h>
#include <stdexcept>
#include <string>

namespace extras{namespace cmex{

    //! group of thread-safe mxArray pointers
    //! useful for capturing all of the arguments passed to a mex function and storing them in a list or queue
    class mxArrayGroup{
        size_t nArrays=0;
        mxArray ** pArray=nullptr;
    public:

        //! create empty array group
        mxArrayGroup(){}; //nothing needed

        //! create arrag group with nA elements
        //! all mxArray elements will be nullptr
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

        //! set a specific element of the array group
        //! NOTE: the previous array at element n will be destroyed
        //! *A will also be converted to a persistent array
        //! *A should not be deleted by any other functions after associating with
        //! the array group
        void setArray(size_t n, mxArray* A){
            if(n>=nArrays){
                throw(std::runtime_error("index exceeds ArrayGroup size"));
            }
            mexMakeArrayPersistent(A);
            mxDestroyArray(pArray[n]);
            pArray[n] = A;
        }

        //! set a specific element of the array group
        //! NOTE: the previous array at element n will be destroyed
        //! *A will also be copied to a new persistent array
        void setArray(size_t n, const mxArray* A){
            if(n>=nArrays){
                throw(std::runtime_error("index exceeds ArrayGroup size"));
            }
            mxDestroyArray(pArray[n]);
            pArray[n] = mxDuplicateArray(A);
            mexMakeArrayPersistent(pArray[n]);
        }

		//! reset using non-const mxArray**
		//! takes ownwership of array and makes all of the mxArray* persistent
		void setFrom(size_t nA, mxArray** pA) {
			for (size_t n = 0; n < nArrays; ++n) {
				mxDestroyArray(pArray[n]);
			}
			delete[] pArray;
			pArray = new mxArray*[nA];
			nArrays = nA;
			for (size_t n = 0; n < nArrays; ++n) {
				pArray[n] = pA[n];
				mexMakeArrayPersistent(pArray[n]);
			}
		}

		//! construct fron non-const mxArray**
		//! takes ownwership of array and makes all of the mxArray* persistent
        //! DO NOT DESTROY mxArray* contained in pA after passing to mxArrayGroup
		mxArrayGroup(size_t nA, mxArray** pA) {
			nArrays = nA;

			if (nA == 0) {
				pArray = nullptr;
				return;
			}

			pArray = new mxArray*[nA];
			for (size_t n = 0; n<nA; ++n) {
				pArray[n] =pA[n];
				mexMakeArrayPersistent(pArray[n]);
			}
		}

        //! number of arrays in the group
        size_t size() const{return nArrays;}

        //! return array of const mxArray*
        operator const mxArray**() const {return const_cast<const mxArray**>(pArray);}

        //! get const array at index n
        const mxArray* getArray(size_t n) const{
            if(n>=nArrays){
                throw(std::runtime_error(
                    std::string("index exceeds ArrayGroup size n=")+std::to_string(n)
                ));
            }
            return pArray[n];
        }

		//! get const array at index n
		const mxArray* getConstArray(size_t n) const {
			if (n >= nArrays) {
				throw(std::runtime_error(
					std::string("index exceeds ArrayGroup size n=") + std::to_string(n)
				));
			}
			return pArray[n];
		}

        //! get array at index n
        mxArray* operator[](size_t n) const{
            if(n>=nArrays){
                throw(std::runtime_error(
                    std::string("index exceeds ArrayGroup size n=")+std::to_string(n)
                ));
            }
            return pArray[n];
        }

        //! copy the array group to an standard array of mxArray pointers
        //! use this to pass arrays out of a mex function
        //! NOTE: array group elements are copied, so plhs[] will not be persistent
        void copyTo(size_t nlhs, mxArray** plhs) const{
            if(nlhs>nArrays){
                throw(std::runtime_error("number of outputs exceeds size of mxArrayGroup"));
            }
            for(size_t n=0;n<nlhs;++n){
                plhs[n] = mxDuplicateArray(pArray[n]);
            }
        }

        //! create array group from standard array of mxArray pointers
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

		// movers
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

        //! Copy by value
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

    /*// concatenate two arrays
    mxArrayGroup cat(mxArrayGroup A, const mxArrayGroup& B){
        A.cat(B);
        return A;
    }*/

}}
