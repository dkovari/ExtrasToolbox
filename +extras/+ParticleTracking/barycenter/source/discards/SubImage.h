#pragma once
#include <mex.h>
#include <stdexcept>
#include <string>
#include "mxClassIDhelpers.hpp"


class SubImage{
protected:
    unsigned char* Data;
    size_t H;
    size_t W;
    size_t Stride;
    mxClassID classID;
    size_t ElemSize;
public:
    SubImage(void* data, mxClassID cid, size_t h, size_t w, size_t stride):
    Data((unsigned char* )data),
    H(h),
    W(w),
    Stride(stride),
    classID(cid)
    {ElemSize = elementsize(cid);};

    /*SubImage(void* data, mxClassID cid, size_t h, size_t w):
    Data((unsigned char* )data),
    H(h),
    W(w),
    Stride(h),
    classID(cid)
    {ElemSize = elementsize(cid);};*/

    void* data() const { return (void*)Data;}
    inline size_t nCols() const { return W; }
	inline size_t nRows() const { return H; }
	inline size_t stride() const { return Stride; }
    inline mxClassID ClassID() const {return classID;}

    //return address of data element
    //dereference using whatever type you think the data is
    inline void* ElementPtr(size_t r, size_t c) const{
        using namespace std;
        if(r>=H || c>= W){
            string err="SubImage::Element(";
            err += to_string(r);
            err+=",";
            err+= to_string(c);
            err+= ") Index out of bounds\nRange:[H=";
            err+=to_string(H);
            err+=",W=";
            err+=to_string(W);
            err+="]\n";
            throw(std::runtime_error(err));
        }

        return &( Data[r*ElemSize + c*Stride*ElemSize] );
    }

    inline double operator()(size_t r, size_t c) const{
        using namespace std;
        if(r>=H || c>= W){
            string err="SubImage::operator(";
            err += to_string(r);
            err+=",";
            err+= to_string(c);
            err+= ") Index out of bounds\nRange:[H=";
            err+=to_string(H);
            err+=",W=";
            err+=to_string(W);
            err+="]\n";
            throw(std::runtime_error(err));
        }

        void* dataPtr = ElementPtr(r,c);

        switch(classID)
        {
    	case mxDOUBLE_CLASS:
    		return *((double*)dataPtr);
    	case mxSINGLE_CLASS:
            return *((float*)dataPtr);
    	case mxINT8_CLASS:
            return *((int8_t*)dataPtr);
    	case mxUINT8_CLASS:
            return *((uint8_t*)dataPtr);
    	case mxINT16_CLASS:
            return *((int16_t*)dataPtr);
    	case mxUINT16_CLASS:
            return *((uint16_t*)dataPtr);
    	case mxINT32_CLASS:
            return *((int32_t*)dataPtr);
    	case mxUINT32_CLASS:
            return *((uint32_t*)dataPtr);
    	case mxINT64_CLASS:
            return *((int64_t*)dataPtr);
    	case mxUINT64_CLASS:
            return *((uint64_t*)dataPtr);
    	default:
    		throw(std::runtime_error("SubImage::operator(): mxClassID not implemented"));
    	}
    }

};

SubImage mx2SubImage(const mxArray* src, size_t r0, size_t c0, size_t H, size_t W){
    if(mxGetNumberOfDimensions(src)!=2){
        throw(std::runtime_error("convertMx2SubDouble: src must be a matrix"));
    }
    if(mxIsComplex(src)){
        throw(std::runtime_error("convertMx2SubDouble: src is complex, it must be real."));
    }
    if(!mxIsNumeric(src)){
        throw(std::runtime_error("convertMs2SubDouble: src is not numeric"));
    }

    size_t height = mxGetM(src);
    size_t width = mxGetN(src);
    size_t elsz = mxGetElementSize(src);

    if(H+r0>height){
        throw(std::runtime_error("mx2SubImage() H+r0>height, out of bounds"));
    }
    if(W+c0>width){
        throw(std::runtime_error("mx2SubImage() W+c0>width, out of bounds"));
    }

    return SubImage( &( ((unsigned char*)mxGetData(src))[r0*elsz+c0*elsz*height] ),mxGetClassID(src), H, W, height);
}
