/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <mex.h>
#include "mxobject.hpp"
#include <extras/ArrayBase.hpp>

namespace extras {namespace cmex {

	class DynamicTypeMxArray : public MxObject, virtual public DynamicTypeArrayBase{
	protected:
		template <typename M> void copyFrom(const ArrayBase<M>& src) {
			bool wasPersistent = isPersistent();

			own(mxCreateNumericArray(src.ndims(), src.dims().data(), type2ClassID<M>(), mxREAL));
			valueCopy((M*)mxGetData(getmxarray()), src.getdata(), src.numel());
			changeValueType(mxType());

			if (wasPersistent) {
				makePersistent();
			}
		}

		//! create deep copy of object
		virtual void copyFrom(const MxObject& src) {
			bool wasPersistent = isPersistent();
			if (src.getmxarray() == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL),false);
			}
			else{ //if same type, use MxObject's copyFrom
				MxObject::copyFrom(src);
			}
			changeValueType(mxType());

			if (wasPersistent) {
				makePersistent();
			}
		}

		// try to move from MxObject, if different type perform copy
		virtual void moveFrom(MxObject& src) {
			bool wasPersistent = isPersistent();
			if (src.getmxarray() == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL),false);
			}
			else{
				MxObject::moveFrom(src);
			}
			changeValueType(mxType());

			if (wasPersistent) {
				makePersistent();
			}
		}

		//! set from const mxArray*
		//! creates a link to the const mxArray* until calling (non-const) operator[]() or (non-const) getdata(),
		//! at which point the array is duplicated
		virtual void setFromConst(const mxArray* psrc, bool persist) {
			bool wasPersistent = isPersistent();
			if (psrc == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL),false);
			}
			else {
				//own(mxDuplicateArray(psrc));
				MxObject::setFromConst(psrc, persist);
			}
			changeValueType(mxType());

			if (wasPersistent) {
				makePersistent();
			}
		}

		//! set from (non-const) mxArray*
		//! NOTE: after calling psrc will still be valid (i.e. not managed by the NumericArray)
		//! so it is your job to delete it.
		virtual void setFrom(mxArray* psrc, bool persist) {
			bool wasPersistent = isPersistent();
			if (psrc == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL),false);
			}
			else{
				//own(mxDuplicateArray(psrc));
				MxObject::setFrom(psrc, persist);
			}
			
			changeValueType(mxType());

			if (wasPersistent) {
				makePersistent();
			}
		}

		//! set from (non-const) mxArray*
		//! NumericArray will take ownership of the mxArray*
		//! DO NOT DELETE PSRC. DO NOT EXPECT PSRC TO BE VALID AFTER CALLING!
		virtual void setOwn(mxArray* psrc, bool persist) {
			bool wasPersistent = isPersistent();
			if (psrc == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL),false);
			}
			else{
				MxObject::setOwn(psrc, persist);
			}
			changeValueType(mxType());
			if (wasPersistent) {
				makePersistent();
			}
		}

	public:
		//////////////////////////////////////////////////////////////////////////////////
		// Methods from ArrayBase that must be defined

		virtual size_t numel() const { return MxObject::numel(); } ///< number of elements, inhereted from MxObject
		virtual bool isempty() const { return MxObject::isempty(); } ///< is array empty, inhereted from MxObject
		virtual size_t ndims() const { return MxObject::ndims(); } ///< number of dimensions

		virtual size_t nRows() const { return mxGetM(getmxarray()); } //number of rows
		virtual size_t nCols() const { return mxGetN(getmxarray()); } //number of columns, is nd-array, nCols=numel/nRows
		virtual std::vector<size_t> dims() const { return MxObject::size(); } //returns size of data
		virtual std::vector<size_t> size() const { return MxObject::size(); } //returns size of data

		 //! get const pointer to raw data array
		const void* untyped_data() const {
			if (getmxarray() == nullptr) {
				return nullptr;
			}

			return mxGetData(getmxarray());
		};

		//! get (non-const) pointer to raw data array
		//! if array was originally set from const mxArray* then it will be duplicated
		void* untyped_data() {
			if (getmxarray() == nullptr) {
				return nullptr;
			}

			if (isConst()) { //was const, need to make a copy so that we can edit values
				bool wasPersistent = isPersistent();
				own(mxDuplicateArray(getmxarray()));
				if (wasPersistent) {
					makePersistent();
				}
			}

			return mxGetData(getmxarray());
		}


		////////////////////////////////////
		// destructor
		virtual ~DynamicTypeMxArray() {};

		//////////////////////////////////
		// Constructors

		//! defaul constructor
		DynamicTypeMxArray() :DynamicTypeArrayBase(vt_double), MxObject() {
			own(mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL));
		};


		////////////////////////////////
		//// mxArray* assignments

		//! set from const mxArray*, with optional ability to set persistent flag
		virtual DynamicTypeMxArray& set(const mxArray* psrc, bool isPersist = false) {
			setFromConst(psrc, isPersist);
			return *this;
		}

		//! set from const mxArray*
		virtual DynamicTypeMxArray& operator=(const mxArray* psrc) {
			set(psrc);
			return *this;
		}

		//! set from (non-const) mxArray*, with optional ability to set persistent flag
		virtual DynamicTypeMxArray& set(mxArray* psrc, bool isPersist = false) {
			setFrom(psrc, isPersist);
			return *this;
		}

		//! set from (non-const) mxArray*
		virtual DynamicTypeMxArray& operator=(mxArray* psrc) {
			set(psrc);
			return *this;
		}

		//! set non-const mxArray*, giving MxObject full ownership.
		//! CATION: Do not delete psrc after calling this method!
		virtual DynamicTypeMxArray& own(mxArray* psrc, bool isPersist = false) {
			setOwn(psrc, isPersist);
			return *this;
		}

		//! construct vector of size num
		DynamicTypeMxArray(size_t num) :DynamicTypeArrayBase(vt_double), MxObject(mxCreateDoubleMatrix(num, 1, mxREAL)) {};
		DynamicTypeMxArray(size_t rows, size_t cols, bool setZeros = false) :DynamicTypeArrayBase(vt_double), MxObject(mxCreateDoubleMatrix(rows, cols, mxREAL)) {};
		DynamicTypeMxArray(const std::vector<size_t>& dim, bool setZeros = false) :DynamicTypeArrayBase(vt_double), MxObject(mxCreateNumericArray(dim.size(),dim.data(),mxDOUBLE_CLASS,mxREAL)) {};


		//! copy from MxObject
		DynamicTypeMxArray(const MxObject& src): DynamicTypeArrayBase(mxClassID2valueType(src.mxType())) {
			copyFrom(src);
		}

		//! Construct from Generic ArrayBase
		template <typename M> DynamicTypeMxArray(const ArrayBase<M>& src): DynamicTypeArrayBase(type2valueType<M>()){
			copyFrom(src);
		}

		//! Assign from Generic ArrayBase;
		template <typename M> DynamicTypeMxArray& operator=(const ArrayBase<M>& src) {
			copyFrom(src);
			return (*this);
		}

		//! Move Construct from MxObject
		DynamicTypeMxArray(MxObject&& src) : DynamicTypeArrayBase(mxClassID2valueType(src.mxType())) {
			moveFrom(src);
		}

		//! Construct from const mxArray*
		//! NumericArray will not take ownership of the array
		DynamicTypeMxArray(const mxArray* psrc, bool persist = false) : DynamicTypeArrayBase(vt_double) {
			setFromConst(psrc, persist);
		}

		//! Construct from (non-const) mxArray*
		//! NumericArray will not take ownership of the array
		//! therefore it is your job to delete the array after NumericArray is done with it.
		DynamicTypeMxArray(mxArray* psrc, bool persist = false) : DynamicTypeArrayBase(vt_double) {
			setFrom(psrc, persist);
		}


	};

	//! Typed NumericArray Wrapper for MxObjects.
	//! Implements the ArrayBase<T> interface
	//! NOTE: When creating integer type array be sure to specify the precision
	//! e.g NumericArray<int32_t> because the type T=int is unsupported.
	//! MATLAB does not have a default integet type therefore it does not make
	//! make sense to include it as an option.
	//! NOTE:
	//! If the
	template <typename T>
	class NumericArray : public DynamicTypeMxArray, virtual public ArrayBase<T> {
	protected:
		template <typename M> void copyFrom(const ArrayBase<M>& src) {
			bool wasPersistent = isPersistent();

			own(mxCreateNumericArray(src.ndims(), src.dims().data(), type2ClassID<T>(), mxREAL));
			valueCopy((T*)mxGetData(getmxarray()), src.getdata(), src.numel());

			if (wasPersistent) {
				makePersistent();
			}
		}


		//! create deep copy of object
		virtual void copyFrom(const MxObject& src) {
			bool wasPersistent = isPersistent();
			if (src.getmxarray() == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, type2ClassID<T>(), mxREAL),false);
			}
			else if (sametype<T>(src)) { //if same type, use MxObject's copyFrom
				MxObject::copyFrom(src);
			}
			else {
				own(mxCreateNumericArray(src.ndims(), src.size().data(), type2ClassID<T>(), mxREAL));
				valueCopy((T*)mxGetData(getmxarray()), src.getmxarray());;
			}
			if (wasPersistent) {
				makePersistent();
			}
			changeValueType(mxType());
		}

		// try to move from MxObject, if different type perform copy
		virtual void moveFrom(MxObject& src) {
			bool wasPersistent = isPersistent();
			if (src.getmxarray() == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, type2ClassID<T>(), mxREAL),false);
			}
			else if (sametype<T>(src)) {
				MxObject::moveFrom(src);
			}
			else {
				NumericArray::copyFrom(src);
			}
			if (wasPersistent) {
				makePersistent();
			}
			changeValueType(mxType());
		}

		//! set from const mxArray*
		//! creates a link to the const mxArray* until calling (non-const) operator[]() or (non-const) getdata(),
		//! at which point the array is duplicated
		virtual void setFromConst(const mxArray* psrc, bool persist) {
			bool wasPersistent = isPersistent();
			if (psrc == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, type2ClassID<T>(), mxREAL),false);
			}
			else if (sametype<T>(psrc)) {
				//own(mxDuplicateArray(psrc));
				MxObject::setFromConst(psrc, persist);
			}
			else { //need to valuecopy
				own(mxCreateNumericArray(mxGetNumberOfDimensions(psrc), mxGetDimensions(psrc), type2ClassID<T>(), mxREAL));
				valueCopy((T*)mxGetData(getmxarray()), psrc);
			}
			if (wasPersistent) {
				makePersistent();
			}
			changeValueType(mxType());
		}

		//! set from (non-const) mxArray*
		//! NOTE: after calling psrc will still be valid (i.e. not managed by the NumericArray)
		//! so it is your job to delete it.
		virtual void setFrom(mxArray* psrc, bool persist) {
			bool wasPersistent = isPersistent();
			if (psrc == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, type2ClassID<T>(), mxREAL),false);
			}
			else if (sametype<T>(psrc)) {
				//own(mxDuplicateArray(psrc));
				MxObject::setFrom(psrc, persist);
			}
			else { //need to valuecopy
				own(mxCreateNumericArray(mxGetNumberOfDimensions(psrc), mxGetDimensions(psrc), type2ClassID<T>(), mxREAL));
				valueCopy((T*)mxGetData(getmxarray()), psrc);
			}
			if (wasPersistent) {
				makePersistent();
			}
			changeValueType(mxType());
		}

		//! set from (non-const) mxArray*
		//! NumericArray will take ownership of the mxArray*
		//! DO NOT DELETE PSRC. DO NOT EXPECT PSRC TO BE VALID AFTER CALLING!
		virtual void setOwn(mxArray* psrc, bool persist) {
			bool wasPersistent = isPersistent();
			if (psrc == nullptr) {
				MxObject::setOwn(mxCreateNumericMatrix(0, 0, type2ClassID<T>(), mxREAL),false);
			}
			else if (sametype<T>(psrc)) {
				MxObject::setOwn(psrc, persist);
			}
			else { //need to valuecopy
				own(mxCreateNumericArray(mxGetNumberOfDimensions(psrc), mxGetDimensions(psrc), type2ClassID<T>(), mxREAL));
				valueCopy((T*)mxGetData(getmxarray()), psrc);
				mxDestroyArray(psrc);
			}
			if (wasPersistent) {
				makePersistent();
			}
			changeValueType(mxType());
		}

	public:
		//////////////////////////////////////////////////////////////////////////////////
		// Methods from ArrayBase that must be defined

		virtual size_t numel() const { return MxObject::numel(); } ///< number of elements, inhereted from MxObject
		virtual bool isempty() const { return MxObject::isempty(); } ///< is array empty, inhereted from MxObject
		virtual size_t ndims() const { return MxObject::ndims(); } ///< number of dimensions

		virtual size_t nRows() const { return mxGetM(getmxarray()); } //number of rows
		virtual size_t nCols() const { return mxGetN(getmxarray()); } //number of columns, is nd-array, nCols=numel/nRows
		virtual std::vector<size_t> dims() const { return MxObject::size(); } //returns size of data
		virtual std::vector<size_t> size() const { return MxObject::size(); } //returns size of data

		//! get const pointer to raw data array
		virtual const T* getdata() const {
			if (getmxarray() ==nullptr) {
				return nullptr;
			}

			return (T*)mxGetData(getmxarray());
		};

		//! get (non-const) pointer to raw data array
		//! if array was originally set from const mxArray* then it will be duplicated
		virtual T* getdata() {
			if (getmxarray() == nullptr) {
				return nullptr;
			}

			if (isConst()) { //was const, need to make a copy so that we can edit values
				bool wasPersistent = isPersistent();
				own(mxDuplicateArray(getmxarray()));
				if(wasPersistent){
					makePersistent();
				}
			}

			return (T*)mxGetData(getmxarray());
		}

		//! returns index corresponding to subscript
		virtual size_t sub2ind(const std::vector<size_t>& subs) const {
			return MxObject::sub2ind(subs);
		}

		//! set n-th element
		//! if array was originally set from const mxArray* then it will be duplicated
		virtual T& operator[](size_t index) {
			if (getmxarray() == nullptr) {
				throw(extras::stacktrace_error("NumericArray::operator[](): MxObject is uninitialized (mxptr==nullptr)"));
			}
			if (index > numel()) {
				throw(extras::stacktrace_error("NumericArray::operator[](): index>numel()"));
			}

			T* dat = getdata();
			return dat[index];
		}

		//! get n-th element
		virtual const T& operator[](size_t index) const {
			if (getmxarray() == nullptr) {
				throw(extras::stacktrace_error("NumericArray::operator[](): MxObject is uninitialized (mxptr==nullptr)"));
			}
			if (index > numel()) {
				throw(extras::stacktrace_error("NumericArray::operator[](): index>numel()"));
			}
			const T* dat = getdata();
			return dat[index];
		}

		//! set n-th element
		virtual T& operator()(size_t index) {return (*this)[index];}

		//! get n-th element
		virtual const T& operator()(size_t index) const {return (*this)[index];}

		//! set element [m,n]
		virtual T& operator()(size_t row, size_t col) { return (*this)({ row,col }); }

		//! get element [m,n]
		virtual const T& operator()(size_t row, size_t col) const { return (*this)({ row,col }); }

		//! set element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual T& operator()(const std::vector<size_t>& sub) { return (*this)[sub2ind(sub)]; }

		//! get element at coordinate [x,y,z,...] specified by the vector elementCoord///< get element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual const T& operator()(const std::vector<size_t>& sub) const {return (*this)[sub2ind(sub)];}

		virtual void resize(size_t num) { reshape(num,1); } ///< resize to hold n elements, keep old data, new data set to zeros
		virtual void resize(size_t nRows, size_t nCols) { reshape(nRows, nCols); } ///< resize to hold MxN elements, keep old data, new data set to zeros
		virtual void resize(const std::vector<size_t>& dim) { reshape(dim); } ///< resize to new size

		virtual void resize_nocpy(size_t num) { reshape_nocopy(num, 1); } ///< resize to hold n elements, discard original data
		virtual void resize_nocpy(size_t nRows, size_t nCols) { reshape_nocopy(nRows, nCols); }///< resize to hold MxN elements, discard original data
		virtual void resize_nocpy(const std::vector<size_t>& dim) { reshape_nocopy(dim); } ///< resize to n elements, discard original data

		virtual void resize_clear(size_t rows, size_t cols) { reshape_nocopy(rows,cols); }///< resize to hold MxN elements, set all elements to zero
		virtual void resize_clear(const std::vector<size_t>& dim) { reshape_nocopy(dim); };///< resize to MxNx... elements, set all elements to zero

		////////////////////////////////////
		// destructor
		virtual ~NumericArray() {};

		//////////////////////////////////
		// Constructors

		//! defaul constructor
		NumericArray() :DynamicTypeArrayBase(type2valueType<T>()){
			own(mxCreateNumericMatrix(0, 0, type2ClassID<T>(), mxREAL));
		};

		NumericArray(const NumericArray<T>& src) :DynamicTypeArrayBase(type2valueType<T>()) {
			copyFrom(src);
		}

		template<typename M> NumericArray(const NumericArray<M>& src) : DynamicTypeArrayBase(type2valueType<T>()) {
			copyFrom<M>(src);
		}

		NumericArray(NumericArray<T>&& src) :DynamicTypeArrayBase(type2valueType<T>()) {
			moveFrom(src);
		}

		template<typename M> NumericArray(NumericArray<M>&& src) : DynamicTypeArrayBase(type2valueType<T>()) {
			moveFrom(src);
		}

		//! copy assign;
		template <typename M> NumericArray& operator=(const NumericArray<M>& src) {
			copyFrom<M>(src);
			return (*this);
		}

		//! move assign
		template <typename M> NumericArray& operator=(NumericArray<M>&& src) {
			moveFrom(src);
			return (*this);
		}


		////////////////////////////////
		//// mxArray* assignments

		//! set from const mxArray*, with optional ability to set persistent flag
		virtual NumericArray& set(const mxArray* psrc, bool isPersist = false) {
			setFromConst(psrc, isPersist);
			return *this;
		}

		//! set from const mxArray*
		virtual NumericArray& operator=(const mxArray* psrc) {
			set(psrc);
			return *this;
		}

		//! set from (non-const) mxArray*, with optional ability to set persistent flag
		virtual NumericArray& set(mxArray* psrc, bool isPersist = false) {
			setFrom(psrc, isPersist);
			return *this;
		}

		//! set from (non-const) mxArray*
		virtual NumericArray& operator=(mxArray* psrc) {
			set(psrc);
			return *this;
		}

		//! set non-const mxArray*, giving MxObject full ownership.
		//! CATION: Do not delete psrc after calling this method!
		virtual NumericArray& own(mxArray* psrc, bool isPersist = false) {
			setOwn(psrc, isPersist);
			return *this;
		}

		//! construct vector of size num
		NumericArray(size_t num) :DynamicTypeArrayBase(type2valueType<T>()) {
			resize(num);
		}
		NumericArray(size_t rows, size_t cols, bool setZeros = false) :DynamicTypeArrayBase(type2valueType<T>()) {
			resize(rows,cols);
		}
		NumericArray(const std::vector<size_t>& dim, bool setZeros = false) :DynamicTypeArrayBase(type2valueType<T>()) {
			resize(dim);
		}

		//! copy from MxObject
		NumericArray(const MxObject& src) {
			copyFrom(src);
		}

		//! Construct from Generic ArrayBase
		template <typename M> NumericArray(const ArrayBase<M>& src) {
			copyFrom(src);
		}

		//! Assign from Generic ArrayBase;
		template <typename M> NumericArray& operator=(const ArrayBase<M>& src) {
			copyFrom(src);
			return (*this);
		}

		//! Move Construct from MxObject
		NumericArray(MxObject&& src) :DynamicTypeArrayBase(type2valueType<T>()) {
			moveFrom(src);
		}

		//! Construct from const mxArray*
		//! array will be duplicated when  (non-const) getdata() or (non-const) operator[] are called.
		NumericArray(const mxArray* psrc, bool persist = false) :DynamicTypeArrayBase(type2valueType<T>()) {
			setFromConst(psrc, persist);
		}

		//! Construct from (non-const) mxArray*
		//! NumericArray will not take ownership of the array
		//! therefore it is your job to delete the array after NumericArray is done with it.
		NumericArray(mxArray* psrc, bool persist = false) :DynamicTypeArrayBase(type2valueType<T>()) {
			setFrom(psrc, persist);
		}

		//! overload disp() method
		void disp() const {
			mexPrintf("NumericMatrix: %s [", mxGetClassName(getmxarray()));
			if (getmxarray() == nullptr) {
				mexPrintf("Empty Array (mxArray*=nullptr)]\n");
				return;
			}
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

		///////////////////////////
		//

		//! concatenate with MxObject
		//! just inherits default behavior from MxObject
		NumericArray& concatenate(const MxObject& b, size_t dim) {
			if (isConst()) { //self-manage the data
				selfManage();
			}
			MxObject::concatenate(b, dim);
			return *this;
		}

		//! Concatenate with generic array
		template <typename M>
		NumericArray& concatenate(const ArrayBase<M>& b, size_t dim) {
			if (iscomplex()) { //error on complex data ---------> change in the future
				throw(extras::stacktrace_error("NumericArray::concatenate(): cat for complex data not implemented."));
			}
			if (isConst()) { //self-manage the data
				selfManage();
			}

			////////////////////////////
			// Compute new size
			auto thisSz = size();
			auto thatSz = b.size();
			size_t thisSz_len = thisSz.size();
			size_t thatSz_len = thatSz.size();

			size_t maxDimLen = std::max(std::max(thisSz.size(), thatSz.size()), dim + 1);

			// Loop over array dimensions and determine if sizes are compatible
			thisSz.resize(maxDimLen);
			thatSz.resize(maxDimLen);

			for (size_t j = 0; j < maxDimLen; j++) {

				if (j >= thisSz_len) {
					thisSz[j] = 1;
				}

				if (j >= thatSz_len) {
					thatSz[j] = 1;
				}

				if (j == dim) {
					continue;
				}

				if (thisSz[j] != thatSz[j]) {
					throw(extras::stacktrace_error("incompatible sizes for concatenate"));
				}
			}

			auto newSz = thisSz;
			newSz[dim] = thisSz[dim] + thatSz[dim];

			//////////////////////////////////////////////////
			// Create Temporary array to hold new data

			NumericArray<T> newArray(newSz);

			//////////////////
			// Loop over dimension dim and copy data for all other dimensions

			size_t new_dim = 0; //accumulator for current position in newPtr
			for (size_t d = 0; d < thisSz[dim]; d++) { //first loop over mxptr elements
				//loop over other dims
				for (size_t odim = 0; odim < newSz.size(); odim++) {
					if (odim == dim) { //nothing to do for dim
						continue;
					}
					if (thisSz[odim] == 1) {//nothing to do for singleton dimension
						continue;
					}
					//loop over indecies of odim and copy
					for (size_t od = 0; od < thisSz[odim]; od++) {
						std::vector<size_t> subs(newSz.size(), 0.0); //create subscript array with all zeros
						subs[odim] = od; //index for dimension being copied
						subs[dim] = d; //index for dimension being concatenated

						newArray(subs) = (*this)(subs); //copy values

					}
				}
				new_dim++;
			}

			for (size_t d = 0; d < thatSz[dim]; d++) { //second loop over end_obj elements
				//loop over other dims
				for (size_t odim = 0; odim < newSz.size(); odim++) {
					if (odim == dim) { //nothing to do for dim
						continue;
					}
					if (thatSz[odim] == 1) {//nothing to do for singleton dimension
						continue;
					}
					//loop over indecies of odim and copy
					for (size_t od = 0; od < thatSz[odim]; od++) {
						std::vector<size_t> subs(newSz.size(), 0.0); //create subscript array with all zeros
						subs[odim] = od; //index for dimension being copied
						subs[dim] = d; //index for dimension being concatenated

						auto val = b(subs); //make tmp copy of data

						subs[dim] = new_dim; //change the sub, since newArray has different dim
						newArray(subs) = val; //set value
					}
				}
				new_dim++;
			}

			// Set cleanup

			mxFree(mxGetData(getmxarray()));
			mxSetData(getmxarray(),mxGetData(newArray.getmxarray()));
			mxSetData(newArray.getmxarray(), nullptr);
			mxSetDimensions(getmxarray(), newSz.data(), newSz.size()); //change dimensions

			if (isPersistent()) {
				mexMakeMemoryPersistent(getdata());
			}

			return *this;
		}

	};
}}
