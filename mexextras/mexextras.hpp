#pragma once

#include <mex.h>
#include <vector>
#include <string>
#include <stdexcept>

namespace mex{

    /// TYPE to mxClassID conversion
    // NOTE: when using integer types, you need to include the precision
    // e.g. type2ClassID<int32_t> otherwise you will get mxUNKONWN_CLASS
    // MATLAB does not have a default integer type, and since <int> is not
    // guaranteed to map to a certain bit-depth we won't support it.
    template <typename T> mxClassID type2ClassID(){return mxUNKNOWN_CLASS;} // default to mxUNKNOWN_CLASS unless template type T is a standard numeric type (e.g. double, int8_t, etc.)
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

    ///Determine if mxClassID cID is the same type as the template type T
    template <typename T> bool sametype(mxClassID cID){ return cID==type2ClassID<T>();}

	///Determine if mxClassID of the mxArray* is the same type as the template type T
    template <typename T> bool sametype(const mxArray* mA){return mxGetClassID(mA)==type2ClassID<T>();}

	///get dimensions of the mxArray* as a std::vector
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

	///number of elements in an mxArray*
	size_t numel(const mxArray* mxptr){
		if (mxptr == nullptr) {
			throw(std::runtime_error("numel mxArray* == nullptr, cannot get numel."));
		}
        return mxGetNumberOfElements(mxptr);
    }

	///convert mxArray* with ClassID=mxCHAR_CLASS to a std::string
    std::string getstring(const mxArray* mxptr){
		if (!mxIsChar(mxptr)) {
			throw(std::runtime_error("mex::getstring(): cannot return string from mxArray that is not mxCHAR type."));
		}
		char* ca = mxArrayToString(mxptr);
        std::string out(ca);
		mxFree(ca);
        return out;
    }

    ///copy array
    template<typename T> void valueCopy(T* dst, const T* src, size_t numel){
        memcpy(dst,src,numel*sizeof(T));
    }

    ///copy array by value, element-by-element
    template<typename T, typename M> void valueCopy(T* dst, const M* src, size_t numel){
        //mexPrintf("valueCopy(T*, M*,...)\n");
        for(size_t n=0;n<numel;++n){
            //mexPrintf("src[%d]=%g\n",n,(double)(src[n]));
            dst[n] = (T)(src[n]);
        }
    }

  	///copy array by value
  	// sametype<T>(src_mxClassID) is true, then memcpy is used to copy data between src and dst
  	// otherwise values copied element by element, using standard type conversion
  	template<typename T> void valueCopy(T* dst, const void* src,size_t numel, mxClassID src_mxClassID) {
  		if (sametype<T>(src_mxClassID)) {
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
  				throw(std::runtime_error("cannot perform valueCopy From on non-numeric mxArray"));
  			}
  		}
  	}

    ///copy mxArray by value
    template <typename T> void valueCopy(T* dst, const mxArray* src){
        valueCopy(dst,mxGetData(src),mxGetNumberOfElements(src),mxGetClassID(src));
    }

}
