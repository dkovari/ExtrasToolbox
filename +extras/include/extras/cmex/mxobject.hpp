/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once


#define NOMINMAX //don't use the windows definition of min/max
#include <algorithm> //use min/max from std

#include <mex.h>

#include "mexextras.hpp"

#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <utility>

#include "type2ClassID.hpp"
#include <extras/numeric.hpp>

namespace extras {namespace cmex {
	using std::size_t;

	// Wrapper around mxArray*
	// has the ability to automatically handle mxArray construction and destruction
	// The contained mxArray* (_mxptr) is protected by a mutex lock so the class *should* 
	// be thread safe.
	class MxObject {
	private:
		mutable std::mutex _mxptrMutex; //mutex for thread-safe locking of _mxptr;
		mxArray* _mxptr = nullptr; //mxArray ptr
		bool _managemxptr = true; //flag if data is managed
		bool _isPersistent = false; //flag if data is persistent
		bool _setFromConst = false; //flag if set from const
	protected:

		//! delete mxptr if needed
		//! if not managed, set from constant, or not set (i.e. nullptr) memory is not freed
		//! upon return:
		//!		_mxptr == nullptr
		//!		_mxptr == false
		//!		_mxptr == false
		void deletemxptr_nolock() {
			// free memory if needed
			if (_managemxptr && !_setFromConst && _mxptr != nullptr) {
				mxDestroyArray(_mxptr);
			}

			_mxptr = nullptr;
			_managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
		}

		//! creates a mutex lock on _mxptr before calling deletemxptr_nolock()
		void deletemxptr_withlock() {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
		}

		//! create deep copy of object
		virtual void copyFrom(const MxObject& src) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			if (src._mxptr == nullptr) {
				_mxptr = nullptr;
				_isPersistent = false;
				_setFromConst = false;
			}
			else {
				_mxptr = mxDuplicateArray(src._mxptr);
				_managemxptr = true;
				_setFromConst = false;
				if (src._isPersistent) {
					makePersistent();
				}
			}
		}

		//! move object
		virtual void moveFrom(MxObject& src) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;

			deletemxptr_nolock();
			
			_mxptr = src._mxptr;
			_managemxptr = bool(src._managemxptr);
			_setFromConst = bool(src._setFromConst);
			_isPersistent = bool(src._isPersistent);

			// reset src
			src._managemxptr = false;
			src.deletemxptr_withlock();
		}

		//! set from const mxArray*
		//! creates a const link to psrc
		virtual void setFromConst(const mxArray* psrc, bool persist) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();

			_mxptr = (mxArray*)psrc;
			_managemxptr = false;
			if (psrc == nullptr) {
				_setFromConst = false;
			}
			else {
				_setFromConst = true;
			}
			_isPersistent = persist;
		}

		//! set from (non-const) mxArray*
		//! Creates a link to psrc, but does not delete it
		//! YOU ARE RESPONSIBLE FOR MANAGING THE LIFE OF psrc
		virtual void setFrom(mxArray* psrc, bool persist) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = psrc;
			_managemxptr = false;
			_setFromConst = false;
			_isPersistent = persist;
		}

		//! set with full ownership
		//! MxObject will automatically call mxDestroyArray(psrc) when it is destructed
		//! DO NOT DELETE psrc or expect it to be valid after passing to MxObject
		virtual void setOwn(mxArray* psrc, bool persist) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = psrc;
			_managemxptr = true;
			_setFromConst = false;
			_isPersistent = persist;
		}

		//! if linked mxArray is not managed, creates a copy and manages the new copy
		//! if already managed or mxptr==nullptr then does nothing.
		//! If the array was not managed, and a copy is created, the the previously linked array
		//! is returned in a pair, along with a bool specifying if it was set from const.
		//!
		//!	Return:
		//!		out.first -> mxArray*
		//!		out.second -> bool specifying if out.first shoud be considered "const mxArray*"
		std::pair<mxArray*,bool> selfManage() {

			if (_mxptr == nullptr) { //nullptr, nothing to do
				return std::make_pair<mxArray*,bool>(nullptr,false);
			}

			if (_managemxptr && !_setFromConst) { //already managed, nothing to do
				return std::make_pair<mxArray*, bool>(nullptr, false);
			}

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			mxArray* oldPtr = _mxptr;
			_mxptr = mxDuplicateArray(_mxptr);
			_managemxptr = true;
			_setFromConst = false;
			if (_isPersistent) {
				mexMakeArrayPersistent(_mxptr);
			}

			return std::pair<mxArray*, bool>(oldPtr, bool(_setFromConst));
		}

		//! returns (non-const) mxArray* _mxptr, for internal use by derived classes
		// does not change memory management rules
		mxArray* getmxarray() {
			return _mxptr;
		}

	public:

		//! destroy object
		virtual ~MxObject() { deletemxptr_withlock(); };

		//! make mxArray persistent so that it survives different calls to a mexFunction
		//! if linked mxArray was not managed by MxObject or set from const mxarray* 
		//! then it will be duplicated and the MxObject will no longer be "set from const"
		void makePersistent() {
			if (_isPersistent) {
				return;
			}

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;

			if (!_managemxptr || _setFromConst) {
				_mxptr = mxDuplicateArray(_mxptr);
				_managemxptr = true;
				_setFromConst = false;
			}

			mexMakeArrayPersistent(_mxptr);
			_isPersistent = true;
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Constructors

		//! Defaul Constructor
		//! mxArray will be set to nullptr
		MxObject() : _mxptr(nullptr), _managemxptr(true), _isPersistent(false), _setFromConst(false) {};

		//! Construct by copy
		MxObject(const MxObject& src) {
			copyFrom(src);
		}

		//! Construct by move
		MxObject(MxObject&& src) {
			moveFrom(src);
		}

		//! Construct from const mxArray*
		MxObject(const mxArray* psrc, bool persist = false) {
			setFromConst(psrc, persist);
		}

		//! Construct from (non-const) mxArray*
		MxObject(mxArray* psrc, bool persist = false) {
			setFrom(psrc, persist);
		}

		///////////////////////////////////////////////////////////////////////
		// Set Operators

		//! set by copy
		virtual MxObject& operator=(const MxObject& src) {
			copyFrom(src);
			return *this;
		}

		//! set by move
		virtual MxObject& operator=(MxObject&& src) {
			moveFrom(src);
			return *this;
		}

		//! set from const mxArray*, with optional ability to set persistent flag
		virtual MxObject& set(const mxArray* psrc, bool isPersist = false) {
			setFromConst(psrc, isPersist);
			return *this;
		}

		//! set from (non-const) mxArray*, with optional ability to set persistent flag
		virtual MxObject& set(mxArray* psrc, bool isPersist = false) {
			setFrom(psrc, isPersist);
			return *this;
		}

		//! set non-const mxArray*, giving MxObject full ownership.
		//! CATION: Do not delete psrc after calling this method!
		virtual MxObject& own(mxArray* psrc, bool isPersist = false) {
			setOwn(psrc, isPersist);
			return *this;
		}

		////////////////////////////////////////////////////////////////////////////////////
		// Cast Conversions

		//! return mutable (non-const) mxArray*
		//! returned array will not be persistent so it's ok to pass back to MATLAB
		//! if object is persistent or setFromConst then a copy of mxArray is returned;
		//! otherwise, the linked mxarray is returned and the management flag is changed.
		//! CAUTION:
		//!		Once you call this function it is your responsibility to manage the memory pointed
		//!		by the resulting mxArray. Therefore, if you do not pass it back to MATLAB you MUST
		//!		call mxDeleteArray(...) on the array.
		virtual operator mxArray*() {
			if (_mxptr == nullptr) { //nullptr, just return nullptr
				return nullptr;
			}

			if (_setFromConst || _isPersistent) {
				return mxDuplicateArray(_mxptr);
			}
			_managemxptr = false;
			return _mxptr;
		}

		//! return const mxArray*
		virtual operator const mxArray*() const {
			return _mxptr;
		}

		//! returns const mxArray* linked to data
		//! optionally specify a bool* in which the state of the persistence will be stored.
		virtual const mxArray* getmxarray(bool * wasPersistent = nullptr) const {
			if (wasPersistent != nullptr) {
				*wasPersistent = _isPersistent;
			}
			return _mxptr;
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// MxObject Info

		//! is linked mxarray persistent?
		bool isPersistent() const { return _isPersistent; }

		//! is linked mxarray const?
		bool isConst() const { return _setFromConst; }

		//! is linked mxarray managed by mxobject?
		bool isManaged() const { return _managemxptr; }

		//! std::vector holding dimension of the mxArray object
		virtual std::vector<size_t> size() const { return extras::cmex::size(_mxptr); }

		//! number of elements
		size_t numel() const {
			if (_mxptr == nullptr) {
				return 0;
			}
			return cmex::numel(_mxptr);
		}

		//! true if numeric and complex
		bool iscomplex() const { return isnumeric() && mxIsComplex(_mxptr); }

		//! isscalar
		bool isscalar() const {
			return numel() == 1;
		}

		//! true if mxArray is a struct
		bool isstruct() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsStruct(_mxptr);
		}

		//! true if is numeric
		bool isnumeric() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsNumeric(_mxptr);
		}

		//! true if linked mxArray* is cell array
		bool iscell() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsCell(_mxptr);
		}

		//! true if linked mxArray* is a char array
		bool ischar() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsChar(_mxptr);
		}

		///returns true if ndims < 3
		virtual bool ismatrix() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return  mxGetNumberOfDimensions(_mxptr)<3;
		}

		//! number of dims
		virtual size_t ndims() const {
			if (_mxptr == nullptr) {
				return 0;
			}
			return mxGetNumberOfDimensions(_mxptr);
		}

		//! true if empty
		virtual bool isempty() const {
			if (_mxptr == nullptr) {
				return true;
			}
			return numel() == 0;
		}

		//! return data type held by mxArray pointer
		mxClassID mxType() const {
			if (_mxptr == nullptr) {
				return mxUNKNOWN_CLASS;
			}
			return mxGetClassID(_mxptr);
		}

		//! return 1-d index corresponding to subscript
		size_t sub2ind(std::vector<size_t> subs) const {
			return mxCalcSingleSubscript(_mxptr, subs.size(), subs.data());
		}

		////////////////////////////////////////////////////////////////////////////////
		// MISC Object Management

		//! Take Ownership of array
		//! if linked mxArray is not managed, creates a copy and manages the new copy
		//! if already managed or mxptr==nullptr then does nothing.
		//! If the array was not managed, and a copy is created, the the previously linked array
		//! is returned in a pair, along with a bool specifying if it was set from const.
		//!
		//!	Return:
		//!		out.first -> mxArray*
		//!		out.second -> bool specifying if out.first shoud be considered "const mxArray*"
		virtual std::pair<mxArray*, bool> managearray() {
			return selfManage();
		}

		//! Reshape using matlab-like syntax
		//! Numeric and Char array will reshape the linked mxArray* in place
		//! For struct and cell arrays, the data will be copied to a new mxarray*
		//! and the original pointer will be released/destroyed
		void reshape(const std::vector<size_t>& dims) {
			if (_setFromConst) {
				throw(std::runtime_error("MxObject::reshape(): Cannot reshape MxObject linked to const mxArray*"));
			}

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;

			//! nullptr -> return numeric real double
			if (_mxptr == nullptr) {
				_mxptr = mxCreateNumericArray(dims.size(), dims.data(), mxDOUBLE_CLASS, mxREAL);
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				return;
			}

			//////////////////
			// old and new size
			size_t newNumel = extras::prod(dims);
			size_t oldNumel = numel();

			//! Perform resize
			mxArray* newPtr;
			switch (mxGetClassID(_mxptr)) {
			case mxDOUBLE_CLASS:
			case mxSINGLE_CLASS:
			case mxINT8_CLASS:
			case mxUINT8_CLASS:
			case mxINT16_CLASS:
			case mxUINT16_CLASS:
			case mxINT32_CLASS:
			case mxUINT32_CLASS:
			case mxINT64_CLASS:
			case mxUINT64_CLASS:
				

				if (!mxIsComplex(_mxptr)) { //real data just a simple resize
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxREAL);
					if (newNumel > oldNumel) { // larger, perform copy
						std::memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);
						mxFree(mxGetData(_mxptr)); //free old data
						mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
						mxSetData(newPtr, nullptr); //release pointer from newPtr's control
						mxDestroyArray(newPtr);//delete newPtr
					}
					else if (newNumel < oldNumel) { //smaller, just use reaaloc
						void* _mxptrData = mxRealloc(mxGetData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetData(_mxptr, _mxptrData);
					}
				}
				else { // complex data
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxCOMPLEX);
					if (newNumel > oldNumel) { // larger, perform copy		
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
						std::memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);

						mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
						mxSetData(newPtr, nullptr); //release pointer from newPtr's control
#else
						memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);
						memcpy(mxGetImagData(newPtr), mxGetImagData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);

						mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
						mxSetImagData(_mxptr, mxGetImagData(newPtr)); //set to new data pointer

						mxSetData(newPtr, nullptr); //release pointer from newPtr's control
						mxSetImagData(newPtr, nullptr);
#endif
						mxDestroyArray(newPtr);//delete newPtr
					}
					else if (newNumel < oldNumel) { //smaller, just use reaaloc
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
						void* _mxptrData = mxRealloc(mxGetData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetData(_mxptr,_mxptrData);
#else
						void* _mxptrData = mxRealloc(mxGetData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetData(_mxptr, _mxptrData);

						_mxptrData = mxRealloc(mxGetImagData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetImagData(_mxptr, _mxptrData);
#endif
					}
				}

				mxSetDimensions(_mxptr, dims.data(), dims.size()); //change dim size	

				if (_isPersistent) { //set persistent again if needed
					mexMakeArrayPersistent(_mxptr);
				}
				
				break;
			case mxCHAR_CLASS:
				newPtr = mxCreateCharArray(dims.size(), dims.data());
				std::memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));

				mxFree(mxGetData(_mxptr));
				mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
				mxSetData(newPtr, nullptr); //release pointer from newPtr's control
				mxDestroyArray(newPtr);//delete newPtr
				mxSetDimensions(_mxptr, dims.data(), dims.size()); //change dim size	

				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				break;
			case mxCELL_CLASS:

				if (oldNumel > newNumel) { //need to destroy extras cells
					for (size_t n = newNumel; n < oldNumel; n++) {
						mxDestroyArray(mxGetCell(_mxptr, n));
					}
				}
				mxSetDimensions(_mxptr, dims.data(), dims.size());
				for (size_t n = oldNumel; n < newNumel; n++) { //set new cells to empty
					mxSetCell(_mxptr, n, mxCreateDoubleMatrix(0, 0, mxREAL));
				}
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				break;
			case mxSTRUCT_CLASS:
			{
				//////////////// 
				// Setup Field Names
				size_t nfields = mxGetNumberOfFields(_mxptr);
				const char* * fnames;
				fnames = new const char*[nfields];
				for (size_t n = 0; n < nfields; ++n) {
					fnames[n] = mxGetFieldNameByNumber(_mxptr, n);
				}

				if (oldNumel > newNumel) { //need to destrop extra fields
					for (size_t n = newNumel; n < oldNumel; n++) {
						for (size_t f = 0; f < nfields; f++) {
							mxDestroyArray(mxGetFieldByNumber(_mxptr, n, f));
						}
					}
				}
				mxSetDimensions(_mxptr, dims.data(), dims.size());
				for (size_t n = oldNumel; n < newNumel; n++) { //set new fields to empty
					for (size_t f = 0; f < nfields; f++) {
						mxSetFieldByNumber(_mxptr, n, f, mxCreateDoubleMatrix(0, 0, mxREAL));
					}
				}
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				delete[] fnames;
			}
			break;
			
			default:
				throw(std::runtime_error("reshape not implemented for class"));
			}

		}

		//! Resize
		void reshape(size_t nRows, size_t nCols) {
			reshape({ nRows,nCols });
		}

		//! reshape without copy
		//! numeric array will contain zeros, cell and struct arrays will be have empty elements
		void reshape_nocopy(const std::vector<size_t>& dims) {
			if (_setFromConst) {
				throw(std::runtime_error("MxObject::reshape(): Cannot reshape MxObject linked to const mxArray*"));
			}

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;

			//! nullptr -> return numeric real double
			if (_mxptr == nullptr) {
				_mxptr = mxCreateNumericArray(dims.size(), dims.data(), mxDOUBLE_CLASS, mxREAL);
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				return;
			}

			//////////////////
			// old and new size
			size_t newNumel = extras::prod(dims);
			size_t oldNumel = numel();

			//! Perform resize
			mxArray* newPtr;
			switch (mxGetClassID(_mxptr)) {
			case mxDOUBLE_CLASS:
			case mxSINGLE_CLASS:
			case mxINT8_CLASS:
			case mxUINT8_CLASS:
			case mxINT16_CLASS:
			case mxUINT16_CLASS:
			case mxINT32_CLASS:
			case mxUINT32_CLASS:
			case mxINT64_CLASS:
			case mxUINT64_CLASS:
				if (!mxIsComplex(_mxptr)) { //real data just a simple resize
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxREAL);
					if (newNumel > oldNumel) { // larger, perform copy
						
						//std::memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);
						mxFree(mxGetData(_mxptr)); //free old data
						mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
						mxSetData(newPtr, nullptr); //release pointer from newPtr's control
						mxDestroyArray(newPtr);//delete newPtr
					}
					else if (newNumel < oldNumel) { //smaller, just use realloc
						void* _mxptrData = mxRealloc(mxGetData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetData(_mxptr, _mxptrData);
					}
				}
				else { // complex data
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxCOMPLEX);
					if (newNumel > oldNumel) { // larger, perform copy
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
						//std::memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);

						mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
						mxSetData(newPtr, nullptr); //release pointer from newPtr's control
#else
						//memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);
						//memcpy(mxGetImagData(newPtr), mxGetImagData(_mxptr), mxGetElementSize(_mxptr)*oldNumel);

						mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
						mxSetImagData(_mxptr, mxGetImagData(newPtr)); //set to new data pointer

						mxSetData(newPtr, nullptr); //release pointer from newPtr's control
						mxSetImagData(newPtr, nullptr);
#endif
						mxDestroyArray(newPtr); //delete newPtr
					}
					else if (newNumel < oldNumel) { //smaller, just use reaaloc
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
						void* _mxptrData = mxRealloc(mxGetData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetData(_mxptr, _mxptrData);
#else
						void* _mxptrData = mxRealloc(mxGetData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetData(_mxptr, _mxptrData);

						_mxptrData = mxRealloc(mxGetImagData(_mxptr), mxGetElementSize(_mxptr)*newNumel);
						mxSetImagData(_mxptr, _mxptrData);
#endif
					}
				}

				mxSetDimensions(_mxptr, dims.data(), dims.size()); //change dim size	

				if (_isPersistent) { //set persistent again if needed
					mexMakeArrayPersistent(_mxptr);
				}

				break;
			case mxCHAR_CLASS:
				newPtr = mxCreateCharArray(dims.size(), dims.data());
				//std::memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));

				mxFree(mxGetData(_mxptr));
				mxSetData(_mxptr, mxGetData(newPtr)); //set to new data pointer
				mxSetData(newPtr, nullptr); //release pointer from newPtr's control
				mxDestroyArray(newPtr);//delete newPtr
				mxSetDimensions(_mxptr, dims.data(), dims.size()); //change dim size	

				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				break;
			case mxCELL_CLASS:

				if (oldNumel > newNumel) { //need to destroy extras cells
					for (size_t n = newNumel; n < oldNumel; n++) {
						mxDestroyArray(mxGetCell(_mxptr, n));
					}
				}
				mxSetDimensions(_mxptr, dims.data(), dims.size());
				for (size_t n = oldNumel; n < newNumel; n++) { //set new cells to empty
					mxSetCell(_mxptr, n, nullptr);
				}
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				break;
			case mxSTRUCT_CLASS:
			{
				//////////////// 
				// Setup Field Names
				size_t nfields = mxGetNumberOfFields(_mxptr);
				const char* * fnames;
				fnames = new const char*[nfields];
				for (size_t n = 0; n < nfields; ++n) {
					fnames[n] = mxGetFieldNameByNumber(_mxptr, n);
				}

				if (oldNumel > newNumel) { //need to destrop extra fields
					for (size_t n = newNumel; n < oldNumel; n++) {
						for (size_t f = 0; f < nfields; f++) {
							mxDestroyArray(mxGetFieldByNumber(_mxptr, n, f));
						}
					}
				}
				mxSetDimensions(_mxptr, dims.data(), dims.size());
				for (size_t n = oldNumel; n < newNumel; n++) { //set new fields to empty
					for (size_t f = 0; f < nfields; f++) {
						mxSetFieldByNumber(_mxptr, n, f,nullptr);
					}
				}
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				delete[] fnames;
			}
			break;

			default:
				throw(std::runtime_error("reshape not implemented for class"));
			}
		}

		//! Resize
		void reshape_nocopy(size_t nRows, size_t nCols) {
			reshape_nocopy({ nRows,nCols });
		}

		//! Concatenate with matlab-like syntax
		//! Input:
		//!		end_obj: object to place at end
		//!		dim: dimension along which to concatenate
		virtual MxObject& concatenate(const MxObject& end_obj, size_t dim) {

			if (isConst()) { //this array is const, throw error
				throw("MxObject::concantenate(): cannot cat object is const");
			}

			if (mxGetClassID(end_obj) != mxGetClassID(_mxptr)) {
				throw(std::runtime_error(
					std::string("Cannot concatenate ") + std::string(mxGetClassName(_mxptr))
					+ std::string("with ") + std::string(mxGetClassName(end_obj))));
			}
			if (iscomplex() != end_obj.iscomplex()) {
				throw("concatenate(): either both arrays must be real or both must be complex");
			}

			//////////////////
			// Struct NOT IMPLEMENTED ----> REMOVE WHEN FIXED
			if (mxIsStruct(_mxptr)) {
				throw("MxObject::concatenate(): concatenate with struct not implemented yet.");
			}

			mxArray* end_ptr = (mxArray*)end_obj.getmxarray(); //mxArray* for end_obj
			bool destroy_end_ptr = false;
			if (&end_obj == this) { //cat with self, need to make a copy of the data
				end_ptr = mxDuplicateArray(end_ptr);
				destroy_end_ptr = true;
			}

			////////////////////////////
			// Compute new size

			auto thisSz = size();
			auto thatSz = end_obj.size();
			size_t thisSz_len = thisSz.size();
			size_t thatSz_len = thatSz.size();

			size_t maxDim = std::max(thisSz.size(), thatSz.size());


			// Loop over array dimensions and determine if sizes are compatible
			thisSz.resize(maxDim);
			thatSz.resize(maxDim);
			
			for (size_t j = 0; j < maxDim; j++) {

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
					throw(std::runtime_error("incompatible sizes for concatenate"));
				}
			}

			auto newSz = thisSz;
			newSz[dim] = thisSz[dim] + thatSz[dim];

			//////////////////////////////////////////////////
			// Create Temporary array to hold new data
			mxArray* newPtr = nullptr;
			switch (mxGetClassID(_mxptr)) {
			case mxDOUBLE_CLASS:
			case mxSINGLE_CLASS:
			case mxINT8_CLASS:
			case mxUINT8_CLASS:
			case mxINT16_CLASS:
			case mxUINT16_CLASS:
			case mxINT32_CLASS:
			case mxUINT32_CLASS:
			case mxINT64_CLASS:
			case mxUINT64_CLASS:
			case mxCHAR_CLASS:
			{
				if (mxIsComplex(_mxptr)) {
					newPtr = mxCreateNumericArray(newSz.size(), newSz.data(), mxGetClassID(_mxptr), mxCOMPLEX);
				}
				else {
					newPtr = mxCreateNumericArray(newSz.size(), newSz.data(), mxGetClassID(_mxptr), mxREAL);
				}				
			}
				break;
			case mxCELL_CLASS:
				newPtr = mxCreateCellArray(newSz.size(), newSz.data());
				break;
			case mxSTRUCT_CLASS:
				throw("concentate not implemented for struct");
			default:
				throw(std::runtime_error(std::string("concatenate not implemented for ")+std::string(mxGetClassName(_mxptr))));
			}

			//////////////////
			// Loop over dimension dim and copy data for all other dimensions

			size_t new_dim = 0; //accumulator for current position in newPtr
			for (size_t d = 0; d < thisSz[dim]; d++) { //first loop over mxptr elements
				//loop over other dims
				for (size_t odim = 0; odim < newSz.size(); odim++) {
					if (odim == dim) { //nothing to do for dim
						continue;
					}
					//loop over indecies of odim and copy
					for (size_t od = 0; od < thisSz[odim]; od++) {
						std::vector<size_t> subs(newSz.size(),0.0); //create subscript array with all zeros
						subs[odim] = od; //index for dimension being copied
						subs[dim] = d; //index for dimension being concatenated
						/// NOTE: all other dims will point to first element

						size_t srcIndex = mxCalcSingleSubscript(_mxptr, subs.size(), subs.data());
						subs[dim] = new_dim; //index for cat dim of new array is different
						size_t dstIndex = mxCalcSingleSubscript(newPtr, subs.size(), subs.data());

						if (isnumeric()) { /// COPY Numerics
							if (!mxIsComplex(_mxptr)) { //real
								const uint8_t* src = (const uint8_t*)mxGetData(_mxptr);
								uint8_t* dst = (uint8_t*)mxGetData(newPtr);
								size_t elm_sz = mxGetElementSize(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);
							}
							else { //complex
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
								const uint8_t* src = (const uint8_t*)mxGetData(_mxptr);
								uint8_t* dst = (uint8_t*)mxGetData(newPtr);
								size_t elm_sz = mxGetElementSize(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);
#else
								const uint8_t* src = (const uint8_t*)mxGetData(_mxptr);
								uint8_t* dst = (uint8_t*)mxGetData(newPtr);
								size_t elm_sz = mxGetElementSize(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);

								src = (const uint8_t*)mxGetImagData(_mxptr);
								dst = (uint8_t*)mxGetImagData(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);
#endif
							}
						}
						else if (iscell()) { /// copy cell array
							// move elements into new array
							mxSetCell(newPtr, dstIndex, mxGetCell(_mxptr, srcIndex));
							mxSetCell(_mxptr, srcIndex, nullptr);
						}
					}
				}
				new_dim++;
			}

			for (size_t d = 0; d < thatSz[dim]; d++) { //second loop over end_obj elements
				//loop over other dims
				for (size_t odim = 0; odim < newSz.size(); odim++) {
					if (odim == dim) { //nothing to do for odim==dim, the outloop handles the copy on that dimension
						continue;
					}
					//loop over indecies of odim and copy
					for (size_t od = 0; od < thatSz[odim]; od++) {
						std::vector<size_t> subs(newSz.size(), 0.0); //create subscript array with all zeros
						subs[odim] = od; //index for dimension being copied
						subs[dim] = d; //index for dimension being concatenated
						/// NOTE: all other dims will point to first element

						size_t srcIndex = mxCalcSingleSubscript(end_ptr, subs.size(), subs.data());
						subs[dim] = new_dim; //index for cat dim of new array is different
						size_t dstIndex = mxCalcSingleSubscript(newPtr, subs.size(), subs.data());

						if (isnumeric()) { /// COPY Numerics
							if (!mxIsComplex(end_ptr)) { //real
								const uint8_t* src = (const uint8_t*)mxGetData(end_ptr);
								uint8_t* dst = (uint8_t*)mxGetData(newPtr);
								size_t elm_sz = mxGetElementSize(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);
							}
							else { //complex
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
								const uint8_t* src = (const uint8_t*)mxGetData(end_ptr);
								uint8_t* dst = (uint8_t*)mxGetData(newPtr);
								size_t elm_sz = mxGetElementSize(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);
#else
								const uint8_t* src = (const uint8_t*)mxGetData(end_ptr);
								uint8_t* dst = (uint8_t*)mxGetData(newPtr);
								size_t elm_sz = mxGetElementSize(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);

								src = (const uint8_t*)mxGetImagData(end_ptr);
								dst = (uint8_t*)mxGetImagData(newPtr);
								memcpy(&dst[dstIndex*elm_sz], &src[srcIndex*elm_sz], elm_sz);
#endif
							}
						}
						else if (iscell()) { /// copy cell array
							// copy elements into new array
							mxSetCell(newPtr, dstIndex, mxDuplicateArray(mxGetCell(end_ptr, srcIndex)));
						}
					}
				}
				new_dim++;
			}


			////////////////////
			// Done with copy/merge
			// cleanup

			if (isnumeric()) { //change data ptr
				mxSetDimensions(_mxptr, newSz.data(), newSz.size()); //change dimensions
				if (iscomplex()) { //complex
#if MX_HAS_INTERLEAVED_COMPLEX
					mxFree(mxGetData(_mxptr)); //free old data
					mxSetData(_mxptr, mxGetData(newPtr)); //set new data
					mxSetData(newPtr, nullptr); //reset newPtr				
#else
					mxFree(mxGetData(_mxptr)); //free old data
					mxFree(mxGetImagData(_mxptr)); //free old data
					mxSetData(_mxptr, mxGetData(newPtr)); //set new data
					mxSetImagData(_mxptr, mxGetImagData(newPtr)); //set new data
					mxSetData(newPtr, nullptr); //reset newPtr
					mxSetImagData(newPtr, nullptr); //reset newPtr
#endif
				}
				else { //real
					mxFree(mxGetData(_mxptr)); //free old data
					mxSetData(_mxptr, mxGetData(newPtr)); //set new data
					mxSetData(newPtr, nullptr); //reset newPtr
				}
			}
			else if (iscell()) {
				/// move results from newptr to _mxptr
				for (size_t i = 0; i < mxGetNumberOfElements(_mxptr); i++) {
					mxSetCell(_mxptr, i, mxGetCell(newPtr, i));
					mxSetCell(newPtr, i, nullptr);
				}
			}

			mxDestroyArray(newPtr); //delete newPtr

			if (destroy_end_ptr) { //destroy end_ptr if needed
				mxDestroyArray(end_ptr);
			}

			//reset persistent
			if (_isPersistent) {
				mexMakeArrayPersistent(_mxptr);
			}
			return *this;
		}

		/////////////////////////////////////////////////////////////////
		// specialized assignment/construction

		//! construct from string
		MxObject(const std::string& str) {
			_mxptr = mxCreateString(str.c_str());
			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;
		}

		//! assign from string
		virtual MxObject& operator=(const std::string& str) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = mxCreateString(str.c_str());
			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;

			return *this;
		}

		//! Construct from double scalar
		MxObject(double val) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = mxCreateDoubleScalar(val);

			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;
		}

		//! assign from double
		virtual MxObject& operator=(double val) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = mxCreateDoubleScalar(val);

			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;

			return *this;
		}

		//! assign from numeric vector
		template <typename T>
		MxObject& operator=(const std::vector<T>& vals) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();

			_mxptr = mxCreateNumericMatrix(vals.size(), 1, type2ClassID<T>(), mxREAL);
			valueCopy((T*)mxGetData(_mxptr), vals.data(), vals.size());
			
			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;

			return *this;
		}
	};

	//////////////////////////////////////////////////
	// MxObject Info functions

	//! return data type held by mxArray pointer
	mxClassID mxType(const MxObject& mxo) { return mxo.mxType(); }

	std::vector<size_t> size(const MxObject& mxo) { return mxo.size(); }

	size_t numel(const MxObject& mxo) { return mxo.numel(); }

	bool isstruct(const MxObject& mxo) { return mxo.isstruct(); }
	bool isnumeric(const MxObject& mxo) { return mxo.isnumeric(); }
	bool iscell(const MxObject& mxo) { return mxo.iscell(); }
	bool ischar(const MxObject& mxo) { return mxo.ischar(); }

	std::string getstring(const MxObject& mxo) { return getstring(mxo.getmxarray()); }

	//! concatenate two objects, return as a new object
	MxObject concatenate(MxObject A, const MxObject& B, size_t dim) {
		A.concatenate(B, dim);
		return A;
	}
}}