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


namespace extras {namespace cmex {

	// Wrapper around mxArray*
	// has the ability to automatically handle mxArray construction and destruction
	// The contained mxArray* (_mxptr) is protected by a mutex lock so the class *should* 
	// be thread safe.
	class MxObject {
	protected:
		std::mutex _mxptrMutex; //mutex for thread-safe locking of _mxptr;
		mxArray* _mxptr = nullptr; //mxArray ptr
		std::atomic_bool _managemxptr = true; //flag if data is managed
		std::atomic_bool _isPersistent = false; //flag if data is persistent
		std::atomic_bool _setFromConst = false; //flag if set from const

		/// delete mxptr if needed
		/// if not managed, set from constant, or not set (i.e. nullptr) memory is not freed
		/// upon return:
		///		_mxptr == nullptr
		///		_mxptr == false
		///		_mxptr == false
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

		/// creates a mutex lock on _mxptr before calling deletemxptr_nolock()
		void deletemxptr_withlock() {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
		}

		/// create deep copy of object
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

		/// move object
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

		/// set from const mxArray*
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

		/// set from (non-const) mxArray*
		virtual void setFrom(mxArray* psrc, bool persist) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = psrc;
			_managemxptr = false;
			_setFromConst = false;
			_isPersistent = persist;
		}

		// set with full ownership
		virtual void setOwn(mxArray* psrc, bool persist) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = psrc;
			_managemxptr = true;
			_setFromConst = false;
			_isPersistent = persist;
		}

		/// if linked mxArray is not managed, creates a copy and manages the new copy
		/// if already managed or mxptr==nullptr then does nothing.
		/// If the array was not managed, and a copy is created, the the previously linked array
		/// is returned in a pair, along with a bool specifying if it was set from const.
		///
		///	Return:
		//		out.first -> mxArray*
		//		out.second -> bool specifying if out.first shoud be considered "const mxArray*"
		std::pair<mxArray*,bool> takeOwnership() {

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

	public:

		/// destroy object
		virtual ~MxObject() { deletemxptr_withlock(); };

		/// make mxArray persistent so that it survives different calls to a mexFunction
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

		/// Defaul Constructor
		/// mxArray will be set to nullptr
		MxObject() : _mxptr(nullptr), _managemxptr(true), _isPersistent(false), _setFromConst(false) {};

		/// Construct by copy
		MxObject(const MxObject& src) {
			copyFrom(src);
		}

		/// Construct by move
		MxObject(MxObject&& src) {
			moveFrom(src);
		}

		/// Construct from const mxArray*
		MxObject(const mxArray* psrc, bool persist = false) {
			setFromConst(psrc, persist);
		}

		/// Construct from (non-const) mxArray*
		MxObject(mxArray* psrc, bool persist = false) {
			setFrom(psrc, persist);
		}

		///////////////////////////////////////////////////////////////////////
		// Set Operators

		/// set by copy
		virtual MxObject& operator=(const MxObject& src) {
			copyFrom(src);
			return *this;
		}

		/// set by move
		virtual MxObject& operator=(MxObject&& src) {
			moveFrom(src);
			return *this;
		}

		/// set from const mxArray*, with optional ability to set persistent flag
		virtual MxObject& set(const mxArray* psrc, bool isPersist = false) {
			setFromConst(psrc, isPersist);
			return *this;
		}

		/// set from (non-const) mxArray*, with optional ability to set persistent flag
		virtual MxObject& set(mxArray* psrc, bool isPersist = false) {
			setFrom(psrc, isPersist);
			return *this;
		}

		// set non-const mxArray* with full ownership.
		// do not delete psrc after calling
		virtual MxObject& own(mxArray* psrc, bool isPersist = false) {
			setOwn(psrc, isPersist);
			return *this;
		}

		////////////////////////////////////////////////////////////////////////////////////
		// Cast Conversions

		/// return mutable (non-const) mxArray*
		/// returned array will not be persistent so it's ok to pass back to MATLAB
		/// if object is persistent or setFromConst then a copy of mxArray is returned;
		/// otherwise, the linked mxarray is returned and the management flag is changed.
		/// CAUTION:
		///		Once you call this function it is your responsibility to manage the memory pointed
		///		by the resulting mxArray. Therefore, if you do not pass it back to MATLAB you MUST
		///		call mxDeleteArray(...) on the array.
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

		/// return const mxArray*
		virtual operator const mxArray*() const {
			return _mxptr;
		}

		/// returns const mxArray* linked to data
		/// optionally specify a bool* in which the state of the persistence will be stored.
		virtual const mxArray* getmxarray(bool * wasPersistent = nullptr) const {
			if (wasPersistent != nullptr) {
				*wasPersistent = _isPersistent;
			}
			return _mxptr;
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// MxObject Info

		/// is linked mxarray persistent?
		bool isPersistent() const { return _isPersistent; }

		/// is linked mxarray const?
		bool isConst() const { return _setFromConst; }

		/// is linked mxarray managed by mxobject?
		bool isManaged() const { return _managemxptr; }

		/// std::vector holding dimension of the mxArray object
		virtual std::vector<size_t> size() const { return cmex::size(_mxptr); }

		/// number of elements
		virtual size_t numel() const {
			if (_mxptr == nullptr) {
				return 0;
			}
			return cmex::numel(_mxptr);
		}

		/// true if mxArray is a struct
		bool isstruct() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsStruct(_mxptr);
		}

		/// true if is numeric
		bool isnumeric() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsNumeric(_mxptr);
		}
		bool iscell() const {
			if (_mxptr == nullptr) {
				return false;
			}
			return mxIsCell(_mxptr);
		}
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

		/// number of dims
		virtual size_t ndims() const {
			if (_mxptr == nullptr) {
				return 0;
			}
			return mxGetNumberOfDimensions(_mxptr);
		}

		/// true if empty
		virtual bool isempty() const {
			if (_mxptr == nullptr) {
				return true;
			}
			return numel() == 0;
		}

		/// return data type held by mxArray pointer
		mxClassID mxType() const {
			if (_mxptr == nullptr) {
				return mxUNKNOWN_CLASS;
			}
			return mxGetClassID(_mxptr);
		}

		////////////////////////////////////////////////////////////////////////////////
		// MISC Object Management

		/// Take Ownership of array
		/// if linked mxArray is not managed, creates a copy and manages the new copy
		/// if already managed or mxptr==nullptr then does nothing.
		/// If the array was not managed, and a copy is created, the the previously linked array
		/// is returned in a pair, along with a bool specifying if it was set from const.
		///
		///	Return:
		//		out.first -> mxArray*
		//		out.second -> bool specifying if out.first shoud be considered "const mxArray*"
		virtual std::pair<mxArray*, bool> managearray() {
			return takeOwnership();
		}

		/// Reshape using matlab-like syntax
		virtual void reshape(const std::vector<size_t>& dims) {
			if (_setFromConst) {
				throw(std::runtime_error("MxObject::reshape(): Cannot reshape MxObject linked to const mxArray*"));
			}

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;

			/// nullptr -> return numeric real double
			if (_mxptr == nullptr) {
				_mxptr = mxCreateNumericArray(dims.size(), dims.data(), mxDOUBLE_CLASS, mxREAL);
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				return;
			}

			/// Perform resize
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
				if (!mxIsComplex(_mxptr)) { //real data just a simple copy
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxREAL);
					memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));
				}
				else { // complex data
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxCOMPLEX);
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
					memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));
#else
					memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));
					memcpy(mxGetImagData(newPtr), mxGetImagData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));
#endif
				}
				if (_managemxptr && !_setFromConst) {
					mxDestroyArray(_mxptr);
				}
				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
				break;
			case mxCELL_CLASS:
				newPtr = mxCreateCellArray(dims.size(), dims.data());
				if (_managemxptr && !_setFromConst) { //just move the arrays
					for (size_t n = 0; n<std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)); ++n) {
						mxSetCell(newPtr, n, mxGetCell(_mxptr, n));
						mxSetCell(_mxptr, n, nullptr);
					}
					mxDestroyArray(_mxptr);
				}
				else { //need copy
					for (size_t n = 0; n<std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)); ++n) {
						mxSetCell(newPtr, n, mxDuplicateArray(mxGetCell(_mxptr, n)));
					}
				}
				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
				break;
			case mxSTRUCT_CLASS:
			{
				size_t nfields = mxGetNumberOfFields(_mxptr);
				const char* * fnames;
				fnames = new const char*[nfields];
				for (size_t n = 0; n < nfields; ++n) {
					fnames[n] = mxGetFieldNameByNumber(_mxptr, n);
				}

				newPtr = mxCreateStructArray(dims.size(), dims.data(), nfields, fnames);
				if (_managemxptr && !_setFromConst) { //just move the arrays
					for (size_t n = 0; n<std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)); ++n) {
						for (size_t f = 0; f < nfields; ++f) {
							mxSetFieldByNumber(newPtr, n, f, mxGetFieldByNumber(_mxptr, n, f));
							mxSetFieldByNumber(_mxptr, n, f, nullptr);
						}
					}
					mxDestroyArray(_mxptr);
				}
				else { //need copy
					for (size_t n = 0; n<std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)); ++n) {
						for (size_t f = 0; f < nfields; ++f) {
							mxSetFieldByNumber(newPtr, n, f, mxDuplicateArray(mxGetFieldByNumber(_mxptr, n, f)));
							mxSetFieldByNumber(_mxptr, n, f, nullptr);
						}
					}
				}
				delete[] fnames;

				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
			}
			break;
			case mxCHAR_CLASS:
				newPtr = mxCreateCharArray(dims.size(), dims.data());
				memcpy(mxGetData(newPtr), mxGetData(_mxptr), mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr), mxGetNumberOfElements(newPtr)));
				if (_managemxptr && !_setFromConst) {
					mxDestroyArray(_mxptr);
				}
				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
				break;
			default:
				throw(std::runtime_error("reshape not implemented for class"));
			}

		}

		/// Resize
		virtual void reshape(size_t nRows, size_t nCols) {
			reshape({ nRows,nCols });
		}

		/// reshape without copy
		virtual void reshape_nocopy(const std::vector<size_t>& dims) {
			if (_setFromConst) {
				throw(std::runtime_error("MxObject::reshape(): Cannot reshape MxObject linked to const mxArray*"));
			}

			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;

			/// nullptr -> return numeric real double
			if (_mxptr == nullptr) {
				_mxptr = mxCreateNumericArray(dims.size(), dims.data(), mxDOUBLE_CLASS, mxREAL);
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				return;
			}

			//////////////
			/// Perform resize
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
				if (!mxIsComplex(_mxptr)) { //real data just a simple copy
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxREAL);
				}
				else { // complex data
					newPtr = mxCreateNumericArray(dims.size(), dims.data(), mxGetClassID(_mxptr), mxCOMPLEX);
				}
				if (_managemxptr && !_setFromConst) {
					mxDestroyArray(_mxptr);
				}
				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
				break;
			case mxCELL_CLASS:
				newPtr = mxCreateCellArray(dims.size(), dims.data());
				if (_managemxptr && !_setFromConst) {
					mxDestroyArray(_mxptr);
				}
				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
				break;
			case mxSTRUCT_CLASS:
			{
				size_t nfields = mxGetNumberOfFields(_mxptr);
				const char* * fnames;
				fnames = new const char*[nfields];
				for (size_t n = 0; n < nfields; ++n) {
					fnames[n] = mxGetFieldNameByNumber(_mxptr, n);
				}

				newPtr = mxCreateStructArray(dims.size(), dims.data(), nfields, fnames);
				if (_managemxptr && !_setFromConst) {
					mxDestroyArray(_mxptr);
				}

				delete[] fnames;

				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
			}
			break;
			case mxCHAR_CLASS:
				newPtr = mxCreateCharArray(dims.size(), dims.data());

				if (_managemxptr && !_setFromConst) {
					mxDestroyArray(_mxptr);
				}
				_mxptr = newPtr;
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(newPtr);
				}
				break;
			default:
				throw(std::runtime_error("reshape not implemented for class"));
			}


		}


		/// Resize
		virtual void reshape_nocopy(size_t nRows, size_t nCols) {
			reshape_nocopy({ nRows,nCols });
		}

		/////////////////////////////////////////////////////////////////
		// specialized assignment/construction

		/// construct from string
		MxObject(const std::string& str) {
			_mxptr = mxCreateString(str.c_str());
			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;
		}

		/// assign from string
		virtual MxObject& operator=(const std::string& str) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = mxCreateString(str.c_str());
			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;

			return *this;
		}

		/// assign from double
		virtual MxObject& operator=(double val) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			deletemxptr_nolock();
			_mxptr = mxCreateDoubleScalar(val);

			_setFromConst = false;
			_isPersistent = false;
			_managemxptr = true;

			return *this;
		}

		/// assign from numeric vector
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

	/// return data type held by mxArray pointer
	mxClassID mxType(const MxObject& mxo) { return mxo.mxType(); }

	std::vector<size_t> size(const MxObject& mxo) { return mxo.size(); }

	size_t numel(const MxObject& mxo) { return mxo.numel(); }

	bool isstruct(const MxObject& mxo) { return mxo.isstruct(); }
	bool isnumeric(const MxObject& mxo) { return mxo.isnumeric(); }
	bool iscell(const MxObject& mxo) { return mxo.iscell(); }
	bool ischar(const MxObject& mxo) { return mxo.ischar(); }

	std::string getstring(const MxObject& mxo) { return getstring(mxo.getmxarray()); }

}}