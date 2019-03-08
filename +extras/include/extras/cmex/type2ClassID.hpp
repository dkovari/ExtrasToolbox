/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once
#include <mex.h>

namespace extras{namespace cmex{
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
}}
