#pragma once

#include <mex.h>
#include <vector>
#include <string>
#include <stdexcept>

namespace mex{

    // TYPE to mxClassID conversion
    template <typename T> mxClassID type2ClassID(){return mxUNKNOWN_CLASS;} //default to mxUNKNOWN_CLASS unless template type T is a standard numeric type (e.g. double, int8_t, etc.)
    template<> mxClassID type2ClassID<double>() {return mxDOUBLE_CLASS;}
    template<> mxClassID type2ClassID<float>() {return mxSINGLE_CLASS;}
    template<> mxClassID type2ClassID<int8_t>() {return mxINT8_CLASS;}
    template<> mxClassID type2ClassID<uint8_t>() {return mxUINT8_CLASS;}
    template<> mxClassID type2ClassID<int16_t>() {return mxINT16_CLASS;}
    template<> mxClassID type2ClassID<uint16_t>() {return mxUINT16_CLASS;}
    template<> mxClassID type2ClassID<int32_t>() {return mxINT32_CLASS;}
    template<> mxClassID type2ClassID<uint32_t>() {return mxUINT32_CLASS;}
    template<> mxClassID type2ClassID<int64_t>() {return mxINT64_CLASS;}
    template<> mxClassID type2ClassID<uint64_t>() {return mxUINT64_CLASS;}

    //Determine if mxClassID cID is the same type as the template type T
    template <typename T> bool sametype(mxClassID cID){ return cID==type2ClassID<T>();}

	//Determine if mxClassID of the mxArray* is the same type as the template type T
    template <typename T> bool sametype(const mxArray* mA){return mxGetClassID(mA)==type2ClassID<T>();}

	//get dimensions of the mxArray* as a std::vector
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
    
	//number of elements in an mxArray*
	size_t numel(const mxArray* mxptr){
		if (mxptr == nullptr) {
			throw(std::runtime_error("numel mxArray* == nullptr, cannot get numel."));
		}
        return mxGetNumberOfElements(mxptr);
    }

	//convert mxArray* with ClassID=mxCHAR_CLASS to a std::string
    std::string getstring(const mxArray* mxptr){
		if (!mxIsChar(mxptr)) {
			throw(std::runtime_error("mex::getstring(): cannot return string from mxArray that is not mxCHAR type."));
		}
		char* ca = mxArrayToString(mxptr);
        std::string out(ca);
		mxFree(ca);
        return out;
    }

	//copy array by value
	// sametype<T>(src_mxClassID) is true, then memcpy is used to copy data between src and dst
	// otherwise values copied element by element, using standard type conversion
	template<typename T> void valueCopy(T* dst, void* src,size_t numel, mxClassID src_mxClassID) {
		if (sametype<T>(src_mxClassID)) {
			memcpy((void*)dst, src, numel * sizeof(T));
		}
		else
		{
			switch (src_mxClassID) {
			case mxDOUBLE_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((double*)src)[n];
				}
				break;
			case mxSINGLE_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((float*)src)[n];
				}
				break;
			case mxINT8_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((int8_t*)src)[n];
				}
				break;
			case mxUINT8_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((uint8_t*)src)[n];
				}
				break;
			case mxINT16_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((int16_t*)src)[n];
				}
				break;
			case mxUINT16_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((uint16_t*)src)[n];
				}
				break;
			case mxINT32_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((int32_t*)src)[n];
				}
				break;
			case mxUINT32_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((uint32_t*)src)[n];
				}
				break;
			case mxINT64_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((int64_t*)src)[n];
				}
				break;
			case mxUINT64_CLASS:
				for (size_t n = 0; n < numel; ++n) {
					dst[n] = ((uint64_t*)src)[n];
				}
				break;
			default:
				throw(std::runtime_error("cannot perform valueCopy From on non-numeric mxArray"));
			}
		}
	}
}
