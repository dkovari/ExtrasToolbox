/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <cstddef>
#include  <cstring>


#include <mex.h>
#include <vector>
#include <string>
#include <stdexcept>

#include "type2ClassID.hpp"

#define NOMINMAX
#include <extras/stacktrace_error.hpp>

namespace extras{ namespace cmex{

	//! get dimensions of the mxArray* as a std::vector
    std::vector<size_t> size(const mxArray* mxptr){
        std::vector<size_t> out;
        if(mxptr==nullptr){
            return out;
        }
        out.resize(mxGetNumberOfDimensions(mxptr));

        for(size_t n=0;n<out.size();n++){
            out[n] = mxGetDimensions(mxptr)[n];
        }
        return out;
    }

	//! number of elements in an mxArray*
	size_t numel(const mxArray* mxptr){
		if (mxptr == nullptr) {
			throw(extras::stacktrace_error("numel mxArray* == nullptr, cannot get numel."));
		}
        return mxGetNumberOfElements(mxptr);
    }

	//! convert mxArray* with ClassID=mxCHAR_CLASS to a std::string
    std::string getstring(const mxArray* mxptr){
		if (!mxIsChar(mxptr)) {
			throw(extras::stacktrace_error("extras::cmex::getstring(): cannot return string from mxArray that is not mxCHAR type."));
		}
		char* ca = mxArrayToString(mxptr);
        std::string out(ca);
		mxFree(ca);
        return out;
    }

    //! copy array
    template<typename T> void valueCopy(T* dst, const T* src, size_t numel){
        memcpy(dst,src,numel*sizeof(T));
    }

    //! copy array by value, element-by-element
    template<typename T, typename M> void valueCopy(T* dst, const M* src, size_t numel){
        //mexPrintf("valueCopy(T*, M*,...)\n");
        for(size_t n=0;n<numel;++n){
            //mexPrintf("src[%d]=%g\n",n,(double)(src[n]));
            dst[n] = (T)(src[n]);
        }
    }

  	//! copy array by value
  	//! sametype<T>(src_mxClassID) is true, then memcpy is used to copy data between src and dst
  	//! otherwise values copied element by element, using standard type conversion
  	template<typename T> void valueCopy(T* dst, const void* src,size_t numel, mxClassID src_mxClassID) {
  		if (sametype<T>(src_mxClassID)){
  			memcpy((void*)dst, src, numel * sizeof(T));
  		}
  		else
  		{
  			switch (src_mxClassID) {
  			case mxDOUBLE_CLASS:
                  valueCopy(dst, (double*)src, numel);
  				break;
  			case mxSINGLE_CLASS:
                  valueCopy(dst, (float*)src, numel);
  				break;
  			case mxINT8_CLASS:
                  valueCopy(dst, (int8_t*)src, numel);
  				break;
  			case mxUINT8_CLASS:
                  valueCopy(dst, (uint8_t*)src, numel);
  				break;
  			case mxINT16_CLASS:
                  valueCopy(dst, (int16_t*)src, numel);
  				break;
  			case mxUINT16_CLASS:
                  valueCopy(dst, (uint16_t*)src, numel);
  				break;
  			case mxINT32_CLASS:
  				valueCopy(dst, (int32_t*)src, numel);
  				break;
  			case mxUINT32_CLASS:
  				valueCopy(dst, (uint32_t*)src, numel);
  				break;
  			case mxINT64_CLASS:
  				valueCopy(dst, (int64_t*)src, numel);
  				break;
  			case mxUINT64_CLASS:
  				valueCopy(dst, (uint64_t*)src, numel);
  				break;
  			default:
  				throw(extras::stacktrace_error("cannot perform valueCopy From on non-numeric mxArray"));
  			}
  		}
  	}

    /// copy mxArray by value
    template <typename T> void valueCopy(T* dst, const mxArray* src){
        valueCopy(dst,mxGetData(src),mxGetNumberOfElements(src),mxGetClassID(src));
    }


	//! check if mxSTRUCT has field
	bool hasField(const mxArray* mxptr,const char* fieldname) {
		if (!mxIsStruct(mxptr)) { return false; }
		return mxGetFieldNumber(mxptr, fieldname) >= 0;

	}

	//! check for array equality
	// arrays must be same type, size, and have same elements
	// passing cell or struct throws error
	bool isequal(const mxArray* A, const mxArray* B) {
		if (A == B) {
			return true;
		}

		if (A == nullptr || B == nullptr) {
			return false;
		}
		if (mxGetClassID(A) != mxGetClassID(B)) {
			return false;
		}

		switch (mxGetClassID(A)) {
		case mxUNKNOWN_CLASS:
		case mxCELL_CLASS:
		case mxSTRUCT_CLASS:
		case mxVOID_CLASS:
		case mxFUNCTION_CLASS:
			throw(extras::stacktrace_error("isequal(): invalid mxClassID, arrays must be char, numeric, or logical"));
			break;
		default:
            break;
			//ok
		}

		if (mxGetNumberOfDimensions(A) != mxGetNumberOfDimensions(B)) {
			return false;
		}
		size_t ndim = mxGetNumberOfDimensions(A);
		const size_t* dimA = mxGetDimensions(A);
		const size_t* dimB = mxGetDimensions(B);
		for (size_t n = 0; n < ndim; ++n) {
			if (dimA[n] != dimB[n]) {
				return false;
			}
		}


		uint8_t *dataA = (uint8_t*)mxGetData(A);
		uint8_t *dataB = (uint8_t*)mxGetData(B);
		for (size_t n = 0; n < mxGetNumberOfElements(A)*mxGetElementSize(A); ++n) {
			if (dataA[n] != dataB[n]) {
				return false;
			}
		}

		// made it here, every byte in the data is identical
		return true;

	}

}}
