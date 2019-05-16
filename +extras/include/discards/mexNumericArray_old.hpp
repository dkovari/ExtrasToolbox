#pragma once

#include "GenericArray.hpp"
#include "mxobject.hpp"
#include <algorithm>
#include "mxClassIDhelpers.hpp"

namespace mex {

	// Typed NumericArray Wrapper for MxObjects
	// Implements the TypedArraI<T> interface
	//
	// TypedArray<T> objects provide element access as if the
	// array was a 2d Matrix (with column-major data).
	// See GenericArray.hpp for details
	template<typename T>
	class NumericArray : public MxObject, public TypedArray<T> {
	public:
		//Constructors
		NumericArray() {};

		NumericArray(size_t rows,size_t cols, bool SetZeros){
			resize_nocpy(rows, cols);
			if(SetZeros){
				memset((void*)getdata(),0,numel()*sizeof(T));
			}
		}
		NumericArray(size_t rows, size_t cols){
			resize_nocpy(rows, cols);
		}
		NumericArray(size_t numel){
			resize_nocpy(numel);
		}

		void copyFrom(const MxObject& src) {
			if (!src.isnumeric()) {
				throw(std::runtime_error("Cannot copy mex::NumericArray from non-numeric MxObject"));
			}

			if (sametype<T>(src._mxptr)) {//same type, simple duplicate ok
				MxObject::copyFrom(src);
			}
			else { //different type, perform type conversion
				deletemxptr();

				_mxptr = mxCreateNumericMatrix(mxGetM(src._mxptr), mxGetN(src._mxptr), type2ClassID<T>(), mxREAL);
				_managemxptr = true;
				_isPersistent = false;
				T* data = (T*)mxGetData(_mxptr);
				void* src_data = mxGetData(src._mxptr);
				mex::valueCopy(data, src_data, src.numel(), mxGetClassID(src._mxptr));
			}
		}

		void moveFrom(MxObject&& src) {
			if (!src.isnumeric()) {
				throw(std::runtime_error("Cannot move mex::NumericArray from non-numeric MxObject"));
			}

			if (sametype<T>(src._mxptr)) {//same type, simple duplicate ok
				MxObject::moveFrom(std::move(src));
			}
			else {//different type, copy
				copyFrom(src);
			}
		}

		NumericArray(const MxObject& src){
			this->copyFrom(src);
		}
		NumericArray& operator=(const MxObject& src) {
			this->copyFrom(src);
			return *this;
		}

		NumericArray(MxObject&& src) {
			this->moveFrom(std::move(src));
		}
		NumericArray& operator=(MxObject&& src) {
			this->moveFrom(std::move(src));
			return *this;
		}

		/////////////////////////////////////
		// Construct from NativeArray

		//Copy From native array with same type
		void copyFrom(const TypedArray<T>& src) {
			bool wasPersistent = isPersistent();
			deletemxptr();

			if (src.getdata() !=nullptr) {
				_mxptr = mxCreateNumericMatrix(src.nRows(), src.nCols(), type2ClassID<T>(), mxREAL);
				memcpy(mxGetData(_mxptr), src.getdata(), src.numel() * sizeof(T));
				_managemxptr = true;
			}
			if (wasPersistent) {
				makePersistent();
			}
		}

		//copy from native with different type
		template<typename M> void copyFrom(const TypedArray<M>& src) {
			bool wasPersistent = isPersistent();
			deletemxptr();
			if (src.getdata() !=nullptr) { //copy with type conversion
				_mxptr = mxCreateNumericMatrix(src.nRows(), src.nCols(), type2ClassID<T>(), mxREAL);
				T* data = (T*)mxGetData(_mxptr);
				for (size_t n = 0; n < src.numel(); ++n) {
					data[n] = (T)(src[n]);
				}
				_managemxptr = true;
			}
			if (wasPersistent) {
				makePersistent();
			}
		}

		template<typename M> NumericArray(const TypedArray<M>& src) {
			copyFrom(src);
		}
		template<typename M> NumericArray& operator=(const TypedArray<M>& src) {
			copyFrom(src);
			return *this;
		}

		//=================================================
		// Inherited from TypedArray

		virtual size_t numel() const { return MxObject::numel(); }
		virtual bool isempty() const { return MxObject::isempty(); }
		virtual size_t nRows() const { if (!_mxptr) { return 0; } return mxGetM(_mxptr); }
		virtual size_t nCols() const { if (!_mxptr) { return 0; } return mxGetN(_mxptr); }

		//virtual size_t ndims() const = 0;
		//virtual const size_t* dims() = 0;

		virtual const T* getdata() const { if (!_mxptr) { return nullptr; } return (T*)mxGetData(_mxptr);}
		virtual T* getdata() { if (!_mxptr) { return nullptr; } return (T*)mxGetData(_mxptr); }

		virtual T& operator[](size_t index)
		{
			if (index >= numel()) {
				throw(std::runtime_error("index exceeds number of elements"));
			}
			return  getdata()[index];
		}
		virtual const T& operator[](size_t index) const
		{
			if (index >= numel()) {
				throw(std::runtime_error("index exceeds number of elements"));
			}
			return  getdata()[index];
		}

		virtual T& operator()(size_t index) { return (*this)[index]; }
		virtual const T& operator()(size_t index) const { return (*this)[index]; }

		virtual T& operator()(size_t row, size_t col) {
			if (row >= nRows()) {
				throw(std::runtime_error("row exceeds number of rows"));
			}
			if (col >= nCols()) {
				throw(std::runtime_error("col exceeds number of columns"));
			}

			return getdata()[row + nRows()*col];
		}
		virtual const T& operator()(size_t row, size_t col) const {
			if (row >= nRows()) {
				throw(std::runtime_error("row exceeds number of rows"));
			}
			if (col >= nCols()) {
				throw(std::runtime_error("col exceeds number of columns"));
			}

			return getdata()[row + nRows()*col];
		}

		//virtual T& operator()(size_t row, size_t col...) = 0;
		//virtual const T& operator()(size_t row, size_t col...) const = 0;


		virtual void resize(size_t rows, size_t cols) {
			if (rows == nRows() && cols == nCols()) {
				return;
			}

			bool wasPersistent = isPersistent();
			if (rows*cols == numel() && _managemxptr) { //same size data and selfmanaged
				mxSetM(_mxptr, rows);
				mxSetN(_mxptr, cols);
				return;
			}
			else if (_managemxptr) { //selfmanaged but size changed
				T* data = (T*)mxGetData(_mxptr);
				data = (T*)mxRealloc(data, rows*cols * sizeof(T));
				mxSetData(_mxptr, data);
				mxSetM(_mxptr,rows);
				mxSetN(_mxptr, cols);

				if (wasPersistent) {
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}
			else { //not selfmanaged, make a copy
				mxArray* newMx = mxCreateNumericMatrix(rows, cols, type2ClassID<T>(), mxREAL);

				if(!_mxptr){
					memcpy(mxGetData(newMx), mxGetData(_mxptr), std::min(rows*cols,numel())* sizeof(T));
				}

				_mxptr = newMx;
				_managemxptr = true;
				_isPersistent = false;
				if (wasPersistent) {
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}

		}
		virtual void resize(size_t numel) {
			resize(numel, 1);
		}


		virtual void resize_nocpy(size_t rows, size_t cols) {
			if (rows == nRows() && cols == nCols()) {
				return;
			}

			bool wasPersistent = IsPersistent();
			if (rows*cols == numel() && _managemxptr) { //same size data and selfmanaged
				mxSetM(_mxptr, rows);
				mxSetN(_mxptr, cols);
				return;
			}
			else { //we don't want to copy, so just discard
				deletemxptr();
				_mxptr = mxCreateNumericMatrix(rows, cols, type2ClassID<T>(), mxREAL);
				_managemxptr = true;
				_isPersistent = false;

				if (wasPersistent) {
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}
		}
		virtual void resize_nocpy(size_t numel) {
			resize_nocpy(numel, 1);
		}

		//resize the array and set all elements to zeros
		virtual void resize_clear(size_t rows, size_t cols) {
			resize_nocpy(rows, cols);
			memset((void*)getdata(), 0, rows*cols * sizeof(T));
		}
	};
}
