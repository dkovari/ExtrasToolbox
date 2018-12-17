#pragma once

#include "../ArrayBase.hpp"
#include "mxobject.hpp"
#include <algorithm>
#include "mxClassIDhelpers.hpp"
#include <stdexcept>


namespace extras{namespace cmex {

	/// Typed NumericArray Wrapper for MxObjects.
	/// Implements the ArrayBase<T> interface
	/// NOTE: When creating integer type array be sure to specify the precision
	/// e.g NumericArray<int32_t> because the type T=int is unsupported.
	/// MATLAB does not have a default integet type therefore it does not make
	/// make sense to include it as an option.
	template<typename T>
	class NumericArray : public MxObject, public ArrayBase<T> {
	protected:
		///copy from generic array type
		template <typename M> void copyFrom(const ArrayBase<M>& src){
			bool wasPersistent = isPersistent();
			deletemxptr();

			_mxptr = mxCreateNumericArray(src.ndims(),src.dims().data(),type2ClassID<T>(),mxREAL);

			_managemxptr = true;
			if(wasPersistent){
				mexMakeArrayPersistent(_mxptr);
				_isPersistent = true;
			}
			valueCopy((T*)mxGetData(_mxptr),src.getdata(),src.numel());
		}

		///copy from MxObject
		// If type of MxObject is not numeric, then error is thrown.
		// If type is different that template <T> then copy by value is performed
		// with type-casting on each element.
		virtual void copyFrom(const MxObject& src){

			//check that source was numeric
			if(!src.isnumeric()){
				throw(std::runtime_error("mex::NumericArray.copyFrom: src was not numeric"));
			}

			deletemxptr(); //clear existing data if we are managing it

			const mxArray* src_mxa = src.getmxarray();
			if(sametype<T>(src_mxa)){
				_mxptr = mxDuplicateArray(src_mxa);
				_managemxptr = true;
			}else{
				_mxptr = mxCreateNumericArray(mxGetNumberOfDimensions(src_mxa),mxGetDimensions(src_mxa),type2ClassID<T>(),mxREAL);
				valueCopy((T*)mxGetData(_mxptr), src_mxa);
				_managemxptr = true;
			}

			if(src.isPersistent()){
				makePersistent();
			}
		}

		///move from MxObject
		// If not numeric, error is thrown
		// If type is different than <T> copy by value is used with type casting
		// Otherwise, the source MxObject will release control of its mxArray
		// and pass it to the calling object.
		virtual void moveFrom(MxObject& src){
			//check that source was numeric
			if(!src.isnumeric()){
				throw(std::runtime_error("mex::NumericArray.copyFrom: src was not numeric"));
			}

			deletemxptr(); //clear existing data

			const mxArray* src_mxa = src.getmxarray();
			if(!sametype<T>(src_mxa)){ //different type need to copy
				_mxptr = mxCreateNumericArray(mxGetNumberOfDimensions(src_mxa),mxGetDimensions(src_mxa),type2ClassID<T>(),mxREAL); //create the new mxArray
				valueCopy((T*)mxGetData(_mxptr), src_mxa); //copy elements, by value if necessary
				_managemxptr = true;
				if(src.isPersistent()){
					makePersistent();
				}
			}else{ //same type, just move
				_mxptr = src.releasemxarray(&_managemxptr,&_isPersistent);
			}
		}

	public:
		virtual size_t numel() const {return mxGetNumberOfElements(_mxptr);} //number of elements
		virtual bool isempty() const {return mxGetNumberOfElements(_mxptr)==0;} //is array empty
		virtual size_t nRows() const {return mxGetM(_mxptr);} //number of rows
		virtual size_t nCols() const {return mxGetN(_mxptr);} //number of columns, is nd-array, nCols=numel/nRows

		///returns dimensions of the data
		virtual std::vector<size_t> dims() const{
			std::vector<size_t> out(mxGetNumberOfDimensions(_mxptr));
			memcpy(out.data(),mxGetDimensions(_mxptr),out.size()*sizeof(size_t));
			return out;
		}
		virtual size_t ndims() const {return mxGetNumberOfDimensions(_mxptr);} //number of dimensions

		virtual const T* getdata() const { if (!_mxptr) { return nullptr; } return (T*)mxGetData(_mxptr); } //get pointer to raw data array
		virtual T* getdata() { if (!_mxptr) { return nullptr; } return (T*)mxGetData(_mxptr); } //get pointer to raw data array

		///set n-th element
		virtual T& operator[](size_t index){
			if (index >= numel()) {
				throw(std::runtime_error("index exceeds number of elements"));
			}
			return  getdata()[index];
		}

		///get n-th element
		virtual const T& operator[](size_t index) const{
			if (index >= numel()) {
				throw(std::runtime_error("index exceeds number of elements"));
			}
			return  getdata()[index];
		}

		virtual T& operator()(size_t index){ return (*this)[index]; } //set n-th element
		virtual const T& operator()(size_t index) const { return (*this)[index]; } //get n-th element

		///set element [m,n]
		virtual T& operator()(size_t row, size_t col){return (*this)[row+nRows()*col];}
		virtual const T& operator()(size_t row, size_t col) const {return (*this)[row+nRows()*col];} //get element [m,n]

		///set element at coordinate [x,y,z,...] specified by dim
		virtual T& operator()(const std::vector<size_t>& dim){
			if(dim.empty()){
				throw(std::runtime_error("mex::NumericArray(): dim is empty"));
			}

			if(dim.size()>ndims()){
				throw(std::runtime_error("mex::NumericArray(): dim exceeds array dimensions"));
			}

			auto this_dims = dims();

			size_t pgsz = 1;
			size_t index = dim[0];
			for(size_t n=1;n<dim.size();++n){
				pgsz *= this_dims[n-1];
				index += dim[n]*pgsz;
			}

			return (*this)[index];
		}

		///get element at coordinate [x,y,z,...]
		virtual const T& operator()(const std::vector<size_t>& dim) const{
			if(dim.empty()){
				throw(std::runtime_error("mex::NumericArray(): dim is empty"));
			}

			if(dim.size()>ndims()){
				throw(std::runtime_error("mex::NumericArray(): dim exceeds array dimensions"));
			}

			const size_t* pdims = mxGetDimensions(_mxptr);

			size_t pgsz = 1;
			size_t index = dim[0];
			for(size_t n=1;n<dim.size();++n){
				pgsz *= pdims[n-1];
				index += dim[n]*pgsz;
			}

			return (*this)[index];
		}

		///resize to hold n elements, keep old data
		virtual void resize(size_t numel){
			if(numel==this->numel()&& _managemxptr){
				mxSetM(_mxptr,numel);
				mxSetN(_mxptr,1);
				return;
			}

			bool wasPersistent = isPersistent();
			if(!_managemxptr){ //we aren't managing the data so make a copy
				mxArray* newArray = mxCreateNumericMatrix(numel,1,type2ClassID<T>(),mxREAL);

				if(_mxptr){ //valid mxArray originally stored
					memcpy(mxGetData(newArray),mxGetData(_mxptr),std::min(numel,this->numel())*sizeof(T));
				}

				_mxptr = newArray;
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}

			}else{ // we were managing the data use realloc

				void* newData = mxRealloc(mxGetData(_mxptr),numel*sizeof(T));
				if(wasPersistent){
					mexMakeMemoryPersistent(newData);
				}
				mxSetData(_mxptr,newData);
				mxSetM(_mxptr,numel);
				mxSetN(_mxptr,1);
				if(wasPersistent){ //just to be sure we set everything (MATLAB has poor doc for persistent memory)
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}
		}

		///resize to MxN elements, keep old data
		virtual void resize(size_t nRows, size_t nCols){
			size_t numel = nRows*nCols;

			if(numel==this->numel() && _managemxptr){
				mxSetM(_mxptr,nRows);
				mxSetN(_mxptr,nCols);
				return;
			}

			bool wasPersistent = isPersistent();
			if(!_managemxptr){ //we aren't managing the data so make a copy
				mxArray* newArray = mxCreateNumericMatrix(nRows,nCols,type2ClassID<T>(),mxREAL);

				if(_mxptr){ //valid mxArray originally stored
					memcpy(mxGetData(newArray),mxGetData(_mxptr),std::min(numel,this->numel())*sizeof(T));
				}

				_mxptr = newArray;
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}

			}else{ // we were managing the data use realloc

				void* newData = mxRealloc(mxGetData(_mxptr),numel*sizeof(T));
				if(wasPersistent){
					mexMakeMemoryPersistent(newData);
				}
				mxSetData(_mxptr,newData);
				mxSetM(_mxptr,nRows);
				mxSetN(_mxptr,nCols);
				if(wasPersistent){ //just to be sure we set everything (MATLAB has poor doc for persistent memory)
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}
		}

		///resize ND
		virtual void resize(const std::vector<size_t>& dim){
			size_t numel = 0;
			if(!dim.empty()){
				numel = dim[1];
				for(size_t n=1;n<dim.size();++n){
					numel*=dim[n];
				}
			}

			if(numel==this->numel() && _managemxptr){
				mxSetDimensions(_mxptr,dim.data(),dim.size());
				return;
			}

			bool wasPersistent = isPersistent();
			if(!_managemxptr){ //we aren't managing the data so make a copy

				mxArray* newArray = mxCreateNumericArray(dim.size(),dim.data(),type2ClassID<T>(),mxREAL);

				if(_mxptr){ //valid mxArray originally stored
					memcpy(mxGetData(newArray),mxGetData(_mxptr),std::min(numel,this->numel())*sizeof(T));
				}

				_mxptr = newArray;
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}

			}else{ // we were managing the data use realloc

				void* newData = mxRealloc(mxGetData(_mxptr),numel*sizeof(T));
				if(wasPersistent){
					mexMakeMemoryPersistent(newData);
				}
				mxSetData(_mxptr,newData);
				mxSetDimensions(_mxptr,dim.data(),dim.size());
				if(wasPersistent){ //just to be sure we set everything (MATLAB has poor doc for persistent memory)
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}

		}

		///resize to n elements, discard original data
		virtual void resize_nocpy(size_t numel){
			if(numel==this->numel()&& _managemxptr){
				mxSetM(_mxptr,numel);
				mxSetN(_mxptr,1);
				return;
			}
			bool wasPersistent = isPersistent();
			if(!_managemxptr){ //not managing data, create a new array don't touch old one
				_mxptr = mxCreateNumericMatrix(numel,1,type2ClassID<T>(),mxREAL);
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}else{ //we are managing the data, just destroy old mxArray
				mxDestroyArray(_mxptr);
				_mxptr = mxCreateNumericMatrix(numel,1,type2ClassID<T>(),mxREAL);
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}
		}

		///resize to mxn without copy
		virtual void resize_nocpy(size_t nRows, size_t nCols){
			size_t numel = nRows*nCols;
			if(numel==this->numel()&& _managemxptr){
				mxSetM(_mxptr,nRows);
				mxSetN(_mxptr,nCols);
				return;
			}

			bool wasPersistent = isPersistent();
			if(!_managemxptr){ //not managing data, create a new array don't touch old one
				_mxptr = mxCreateNumericMatrix(nRows,nCols,type2ClassID<T>(),mxREAL);
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}else{ //we are managing the data, just destroy old mxArray
				mxDestroyArray(_mxptr);
				_mxptr = mxCreateNumericMatrix(nRows,nCols,type2ClassID<T>(),mxREAL);
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}

		}

		///resize, discard original data
		virtual void resize_nocpy(const std::vector<size_t>& dim){
			size_t numel = 0;
			if(!dim.empty()){
				numel = dim[1];
				for(size_t n=1;n<dim.size();++n){
					numel*=dim[n];
				}
			}

			if(numel==this->numel() && _managemxptr){
				mxSetDimensions(_mxptr,dim.data(),dim.size());
				return;
			}

			bool wasPersistent = isPersistent();
			if(!_managemxptr){ //not managing data, create a new array don't touch old one
				_mxptr = mxCreateNumericArray(dim.size(),dim.data(),type2ClassID<T>(),mxREAL);
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}else{ //we are managing the data, just destroy old mxArray
				mxDestroyArray(_mxptr);
				_mxptr = mxCreateNumericArray(dim.size(),dim.data(),type2ClassID<T>(),mxREAL);
				_managemxptr = true;
				if(wasPersistent){
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}


		}

		///resize to MxN elements, set all elements to zero
		virtual void resize_clear(size_t rows, size_t cols){
			bool wasPersistent = isPersistent();

			if(_managemxptr){
				mxDestroyArray(_mxptr);
				_mxptr = nullptr;
			}

			_mxptr = mxCreateNumericMatrix(0,0,type2ClassID<T>(),mxREAL);
			void* newData = mxCalloc(rows*cols,sizeof(T));
			mxSetData(_mxptr,newData);
			mxSetM(_mxptr,rows);
			mxSetN(_mxptr,cols);
			_managemxptr = true;
			if(wasPersistent){
				mexMakeArrayPersistent(_mxptr);
				_isPersistent = true;
			}
		}

		///resize and set all to zero
		virtual void resize_clear(const std::vector<size_t>& dim){
			bool wasPersistent = isPersistent();

			size_t numel = 0;
			if(!dim.empty()){
				numel = dim[1];
				for(size_t n=1;n<dim.size();++n){
					numel*=dim[n];
				}
			}

			if(_managemxptr){
				mxDestroyArray(_mxptr);
				_mxptr = nullptr;
			}

			_mxptr = mxCreateNumericMatrix(0,0,type2ClassID<T>(),mxREAL);
			void* newData = mxCalloc(numel,sizeof(T));
			mxSetData(_mxptr,newData);
			mxSetDimensions(_mxptr,dim.data(),dim.size());
			_managemxptr = true;
			if(wasPersistent){
				mexMakeArrayPersistent(_mxptr);
				_isPersistent = true;
			}
		}

		//////////////////////////////////////
		// Constructors & Assignment

		NumericArray() {};

		NumericArray(size_t numel){
			resize_nocpy(numel);
		}
		NumericArray(size_t rows, size_t cols, bool setZeros = false){
			if(setZeros){
				resize_clear(rows,cols);
			}else{
				resize_nocpy(rows,cols);
			}
		}
		NumericArray(const std::vector<size_t>& dim, bool setZeros = false){
			if(setZeros){
				resize_clear(dim);
			}else{
				resize_nocpy(dim);
			}
		}

		///Copy contructor from MxObject
		NumericArray(const MxObject& src){
			copyFrom(src);
		}

		///Assign from MxObject
		NumericArray& operator=(const MxObject& src){
			copyFrom(src);
			return (*this);
		}

		///Construct from Generic ArrayBase
		template <typename M> NumericArray(const ArrayBase<M>& src){
			copyFrom(src);
		}

		///Assign from Generic ArrayBase;
		template <typename M> NumericArray& operator=(const ArrayBase<M>& src){
			copyFrom(src);
			return (*this);
		}

		///Move Construct from MxObject
		NumericArray(MxObject&& src){
			moveFrom(src);
		}

		///move assign from MxObject
		NumericArray& operator=(MxObject&& src){
			moveFrom(src);
			return (*this);
		}

		void disp() const{
			mexPrintf("NumericMatrix: %s [", mxGetClassName(_mxptr));
			for(size_t n=0;n<ndims()-1;++n){
				mexPrintf("%d x ",dims()[n]);
			}
			mexPrintf("%d]\n",dims()[ndims()-1]);
			if(ndims()<=2){
				for(size_t r=0;r<nRows();++r){
					for(size_t c=0;c<nCols();++c){
						mexPrintf("\t%g",(*this)(r,c));
					}
					mexPrintf("\n");
				}
			}
		}

	};

	template<typename T> void disp(const NumericArray<T>& NA){
		NA.disp();
	}
}
}
