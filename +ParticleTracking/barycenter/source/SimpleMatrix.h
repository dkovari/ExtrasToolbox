#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include<mex.h>
#include "mxClassIDhelpers.hpp"



//assumes column-major data layout (LIKE MATLAB)
template<typename T>
class SimpleMatrix {
protected:
	T* Data;
	size_t H;
	size_t W;
	size_t Stride;
	bool selfManaged;

	virtual void alloc_data() {
		Data = (T*)malloc(W*H*sizeof(T));
		if(!Data){
			throw(std::runtime_error("SimpleMatrix::alloc_data() returned nullptr"));
		}
	}
	virtual void calloc_data() {
		Data = (T*)calloc(W*H,sizeof(T));
		if(!Data){
			throw(std::runtime_error("SimpleMatrix::calloc_data() returned nullptr"));
		}
	}
	virtual void free_data() {
		if(selfManaged){
			free(Data);
		}
	}
	virtual void realloc_data(size_t h, size_t w){

		void* tmp = realloc(Data,h*w*sizeof(T));
		if(!tmp){
			throw(std::runtime_error("SimpleMatrix::realloc_data() Could not realloc data. Probably not enough memory."));
		}
		Data = (T*)tmp;
		H = h;
		W = w;
		Stride = h;
	}
public:
	~SimpleMatrix() {
		free_data();
	}
	SimpleMatrix():
		Data(nullptr),
		H(0),
		W(0),
		Stride(0),
		selfManaged(true)
		{};

	SimpleMatrix(T* data, size_t h, size_t w) :
		Data(data),
		H(h),
		W(w),
		Stride(h),
		selfManaged(false)
	{};

	SimpleMatrix(T* data, size_t h, size_t w, size_t stride ) :
		Data(data),
		H(h),
		W(w),
		Stride(stride),
		selfManaged(false)
	{};

	SimpleMatrix(size_t h, size_t w, bool ZeroData=false) :
		Data(nullptr),
		H(h),
		W(w),
		Stride(h),
		selfManaged(true)
	{
		if (ZeroData) {
			calloc_data();
		}
		else {
			alloc_data();
		}
	}

	//move constructors
	SimpleMatrix(SimpleMatrix&& in) {
		Data = in.Data;
		H = in.H;
		W = in.W;
		Stride = in.Stride;
		selfManaged = in.selfManaged;

		in.Data = nullptr;
	}

	SimpleMatrix& operator=(SimpleMatrix&& in) {
		free_data();

		Data = in.Data;
		H = in.H;
		W = in.W;
		Stride = in.Stride;
		selfManaged = in.selfManaged;

		in.Data = nullptr;
		return *this;
	}

	inline T* data() const {return Data;}
	inline size_t nCols() const { return W; }
	inline size_t nRows() const { return H; }
	inline size_t stride() const { return Stride; }

	inline T& operator()(size_t r, size_t c) {
		if (r < 0 || r >= H || c < 0 || c >= W) {
			mexPrintf("r=%d, c=%d, nRows=%d,nCols=%d\n",r,c,H,W);
			throw(std::runtime_error("SimpleMatrix []: Index out of range"));
		}

		return Data[r + c*Stride];
	}
	inline const T& operator()(size_t r, size_t c) const {
		if (r < 0 || r >= H || c < 0 || c >= W) {
			mexPrintf("r=%d, c=%d, nRows=%d,nCols=%d\n",r,c,H,W);
			throw(std::runtime_error("SimpleMatrix const[]: Index out of range"));
		}

		return Data[r + c*Stride];
	}

	inline T getValue(size_t r, size_t c) const{
		if (r < 0 || r >= H || c < 0 || c >= W) {
			mexPrintf("r=%d, c=%d, nRows=%d,nCols=%d\n",r,c,H,W);
			throw(std::runtime_error("SimpleMatrix const[]: Index out of range"));
		}
		return Data[r + c*Stride];
	}

	void resize(size_t h, size_t w){
		if(!selfManaged){
			throw(std::runtime_error("SimpleMatrix::resize(): Matrix is not selfManaged, cannot resize."));
		}
		realloc_data(h,w);
	}

	void setData(T* data, size_t h, size_t w, size_t stride){
		free_data();
		Data = data;
		H= h;
		W = w;
		Stride = stride;
		selfManaged=false;
	}
	void setData(T* data, size_t h, size_t w){
		setData(data, h, w, h);
	}
	
	inline T* ElementPtr(size_t r, size_t c) const{
        using namespace std;
        if(r>=H || c>= W){
            string err="SimpleMatrix::Element(";
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

        return &( Data[r + c*Stride] );
    }

};

//copy SimpleMatrix to a c array
//assumes colum-major data
template<typename T>
void copymat(T* dst,const SimpleMatrix<T>& src,size_t dst_stride){
	if(src.nRows()==src.stride()){
		memcpy(dst,src.data(),src.nRows()*src.nCols()*sizeof(T));
	}
	else{
		for(size_t c=0;c<src.nCols();c++){
			for(size_t r=0;r<src.nRows();r++){
				dst[r+c*dst_stride] = src(r,c);
			}
		}
	}

}

//copy SimpleMatrix to a c array
//assume stride of dst == src.nRows()
//assumes colum-major data
template<typename T>
void copymat(T* dst,const SimpleMatrix<T>& src){
	copymat(dst,src,src.nRows());
}

//Plot image
template<typename T>
void imagesc(const SimpleMatrix<T>& img){
    mxArray* mxImg = mxCreateNumericMatrix(img.nRows(), img.nCols(),type2ClassID(typeid(T)),mxREAL);
    copymat((T*)mxGetData(mxImg),img);
    //mexEvalString("figure()");
    mxArray* err = mexCallMATLABWithTrap(0,nullptr,1,&mxImg,"imagesc");

    if(err){
        mexPrintf("Error occured while trying to display image\n");
        //mexEvalString("pause()");
    }
    mxDestroyArray(mxImg);
}
