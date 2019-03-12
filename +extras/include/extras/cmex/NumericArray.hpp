/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "MxObject.hpp"
#include <extras/ArrayBase.hpp>

namespace extras {namespace cmex {

	/// Typed NumericArray Wrapper for MxObjects.
	/// Implements the ArrayBase<T> interface
	/// NOTE: When creating integer type array be sure to specify the precision
	/// e.g NumericArray<int32_t> because the type T=int is unsupported.
	/// MATLAB does not have a default integet type therefore it does not make
	/// make sense to include it as an option.
	template <typename T>
	class NumericArray : public MxObject, virtual public ArrayBase<T> {
	protected:
		template <typename M> virtual void copyFrom(const ArrayBase<M>& src) {
			bool wasPersistent = isPersistent();

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock(); //delete old data
			_mxptr = mxCreateNumericArray(src.ndims(), src.dims().data(), type2ClassID<T>(), mxREAL);
			valueCopy((T*)mxGetData(_mxptr), src.getdata(), src.numel());

			_managemxptr = true;
			if (wasPersistent) {
				mexMakeArrayPersistent(_mxptr);
				_isPersistent = true;
			}
		}

		virtual void copyFrom(const NumericArray<T>& src) {
			bool wasPersistent = _isPersistent;
			MxObject::copyFrom(src);
			if (wasPersistent) {
				makePersistent();
			}
		}

		virtual void moveFrom(NumericArray<T>& src) {
			bool wasPersistent = _isPersistent;
			MxObject::moveFrom(src);
			if (wasPersistent) {
				makePersistent();
			}
		}
		
		template <typename M> virtual void copyFrom(const NumericArray<M>& src) {
			bool wasPersistent = _isPersistent;
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock(); //delete old data
			_mxptr = mxCreateNumericArray(src.ndims(), src.size().data(), type2ClassID<T>(), mxREAL);
			valueCopy((T*)mxGetData(_mxptr), src.getmxarray());
			_managemxptr = true;
			_setFromConst = false;
			_isPersistent = false;
			if (wasPersistent) {
				makePersistent();
			}
		}

		template <typename M> virtual void moveFrom(NumericArray<M>& src) {
			NumericArray::copyFrom(src);
		}

		/// create deep copy of object
		virtual void copyFrom(const MxObject& src) {
			bool wasPersistent = _isPersistent;
			if (sametype<T>(src)) { //if same type, use MxObject's copyFrom
				MxObject::copyFrom(src);
			}
			else {
				std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
				deletemxptr_nolock(); //delete old data
				_mxptr = mxCreateNumericArray(src.ndims(), src.size().data(), type2ClassID<T>(), mxREAL);
				valueCopy((T*)mxGetData(_mxptr), src.getmxarray());
				_managemxptr = true;
				_setFromConst = false;
				_isPersistent = false;
			}
			if (wasPersistent) {
				makePersistent();
			}
		}

		// try to move from MxObject, if different type perform copy
		virtual moveFrom(MxObject& src) {
			bool wasPersistent = _isPersistent;
			if (sametype<T>(src)) {
				MxObject::moveFrom(src);
			}
			else {
				NumericArray::copyFrom(src);
			}
			if (wasPersistent) {
				makePersistent();
			}
		}

		/// set from const mxArray*
		virtual void setFromConst(const mxArray* psrc, bool persist) {
			bool wasPersistent = _isPersistent;
			if (sametype<T>(psrc)) {
				MxObject::setFromConst(psrc,persist);
			}
			else { //need to valuecopy
				std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
				deletemxptr_nolock(); //delete old data
				_mxptr = mxCreateNumericArray(mxGetNumberOfDimensions(psrc), mxGetDimensions(psrc), type2ClassID<T>(), mxREAL);
				valueCopy((T*)mxGetData(_mxptr), psrc);
				_managemxptr = true;
				_setFromConst = false;
				_isPersistent = false;
			}
			if (wasPersistent) {
				makePersistent();
			}
		}

		/// set from (non-const) mxArray*
		virtual void setFrom(mxArray* psrc, bool persist) {
			bool wasPersistent = _isPersistent;
			if (sametype<T>(psrc)) {
				MxObject::setFrom(psrc, persist);
			}
			else { //need to valuecopy
				std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
				deletemxptr_nolock(); //delete old data
				_mxptr = mxCreateNumericArray(mxGetNumberOfDimensions(psrc), mxGetDimensions(psrc), type2ClassID<T>(), mxREAL);
				valueCopy((T*)mxGetData(_mxptr), psrc);
				_managemxptr = true;
				_setFromConst = false;
				_isPersistent = false;
			}
			if (wasPersistent) {
				makePersistent();
			}
		}

	public:
		//////////////////////////////////////////////////////////////////////////////////
		// Methods from ArrayBase that must be defined

		//virtual size_t numel() const ... ///< number of elements, inhereted from MxObject
		//virtual bool isempty() const ...; ///< is array empty, inhereted from MxObject

		virtual size_t nRows() const { return mxGetM(_mxptr); } //number of rows
		virtual size_t nCols() const { return mxGetN(_mxptr); } //number of columns, is nd-array, nCols=numel/nRows
		virtual std::vector<size_t> dims() const { return size(); } //returns size of data

		/// get const pointer to raw data array
		virtual const T* getdata() const { 
			if (_mpxtr==nullptr) {
				return nullptr;
			}
			return (T*)mxGetData(_mptr);
		};
		/// get (non-const) pointer to raw data array
		virtual T* getdata() {
			if (_mpxtr == nullptr) {
				return nullptr;
			}
			if (_setFromConst) {
				throw(std::runtime_error("NumericArray::getdata(): cannot get non-const pointer to data from MxObject that was set from const."));
			}
			return (T*)mxGetData(_mptr);
		}

		///< set n-th element
		virtual T& operator[](size_t index) {
			if (_mpxtr == nullptr) {
				throw(std::runtime_error("NumericArray::operator[](): MxObject is uninitialized (mxptr==nullptr)"));
			}
			if (_setFromConst) {
				throw(std::runtime_error("NumericArray::operator[](): cannot set data for MxObject that was set from const."));
			}
			if (index > numel()) {
				throw(std::runtime_error("NumericArray::operator[](): index>numel()"));
			}
			T* dat = getdata();
			return dat[index];
		}

		///< get n-th element
		virtual const T& operator[](size_t index) const {
			if (_mpxtr == nullptr) {
				throw(std::runtime_error("NumericArray::operator[](): MxObject is uninitialized (mxptr==nullptr)"));
			}
			if (index > numel()) {
				throw(std::runtime_error("NumericArray::operator[](): index>numel()"));
			}
			T* dat = getdata();
			return dat[index];
		}

		///< set n-th element
		virtual T& operator()(size_t index) {return *this[index];}

		///< get n-th element
		virtual const T& operator()(size_t index) const {return *this[index];}

		///< set element [m,n]
		virtual T& operator()(size_t row, size_t col) {
			size_t subs[2] = { row,col };
			return *this[mxCalcSingleSubscript(_mxptr, 2, subs)];
		} 

		///< get element [m,n]
		virtual const T& operator()(size_t row, size_t col) const {
			size_t subs[2] = { row,col };
			return *this[mxCalcSingleSubscript(_mxptr, 2, subs)];
		}

		///< set element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual T& operator()(const std::vector<size_t>& elementCoord) { 
			return *this[mxCalcSingleSubscript(_mxptr, elementCoord.size(), elementCoord.data())];
		} 
		
		///< get element at coordinate [x,y,z,...] specified by the vector elementCoord///< get element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual const T& operator()(const std::vector<size_t>& elementCoord) const {
			return *this[mxCalcSingleSubscript(_mxptr, elementCoord.size(), elementCoord.data())]; 
		} 

		virtual void resize(size_t num) { reshape(num,1); } ///< resize to hold n elements, keep old data, new data set to zeros
		virtual void resize(size_t nRows, size_t nCols) { reshape(nRows, nCols); } ///< resize to hold MxN elements, keep old data, new data set to zeros
		virtual void resize(const std::vector<size_t>& dim) { reshape(dim); } ///< resize to new size

		virtual void resize_nocpy(size_t numel) { reshape_nocopy(num, 1); } ///< resize to hold n elements, discard original data
		virtual void resize_nocpy(size_t nRows, size_t nCols) { reshape_nocopy(nRows, nCols); }///< resize to hold MxN elements, discard original data
		virtual void resize_nocpy(const std::vector<size_t>& dim) { reshape_nocopy(dim); } ///< resize to n elements, discard original data

		virtual void resize_clear(size_t rows, size_t cols) { reshape_nocopy(rows,cols); }///< resize to hold MxN elements, set all elements to zero
		virtual void resize_clear(const std::vector<size_t>& dim) { reshape_nocopy(dim); };///< resize to MxNx... elements, set all elements to zero

		////////////////////////////////////
		// destructor
		virtual ~NumericArray() {};

		//////////////////////////////////
		// Constructors

		//defaul constructor
		NumericArray() : MxObject() {};

		NumericArray(const NumericArray<T>& src) {
			copyFrom(src);
		}
		
		template<typename M> NumericArray(const NumericArray<M>& src) {
			copyFrom<M>(src);
		}

		NumericArray(NumericArray<T>&& src) {
			moveFrom(src);
		}

		template<typename M> NumericArray(NumericArray<M>&& src) {
			moveFrom<M>(src);
		}

		///copy assign;
		template <typename M> NumericArray& operator=(const NumericArray<M>& src) {
			copyFrom<M>(src);
			return (*this);
		}

		///move assign
		template <typename M> NumericArray& operator=(NumericArray<M>&& src) {
			moveFrom<M>(src);
			return (*this);
		}

		// construct vector of size num
		NumericArray(size_t num) {
			resize(num);
		}
		NumericArray(size_t rows, size_t cols, bool setZeros = false) {
			resize(rows,cols);
		}
		NumericArray(const std::vector<size_t>& dim, bool setZeros = false) {
			resize(dim);
		}

		/// copy from MxObject
		NumericArray(const MxObject& src) {
			copyFrom(src);
		}

		///Construct from Generic ArrayBase
		template <typename M> NumericArray(const ArrayBase<M>& src) {
			copyFrom(src);
		}

		///Assign from Generic ArrayBase;
		template <typename M> NumericArray& operator=(const ArrayBase<M>& src) {
			copyFrom(src);
			return (*this);
		}

		///Move Construct from MxObject
		NumericArray(MxObject&& src) {
			moveFrom(src);
		}

		/// overload disp() method
		void disp() const {
			mexPrintf("NumericMatrix: %s [", mxGetClassName(_mxptr));
			for (size_t n = 0; n<ndims() - 1; ++n) {
				mexPrintf("%zu x ", dims()[n]);
			}
			mexPrintf("%zu]\n", dims()[ndims() - 1]);
			if (ndims() <= 2) {
				for (size_t r = 0; r<nRows(); ++r) {
					for (size_t c = 0; c<nCols(); ++c) {
						mexPrintf("\t%g", (double)(*this)(r, c));
					}
					mexPrintf("\n");
				}
			}
		}

	};
}}