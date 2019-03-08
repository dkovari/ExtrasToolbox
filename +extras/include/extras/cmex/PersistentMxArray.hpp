#pragma once
/* PersistentMxArray.hpp
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
*/

#include <mex.h>

namespace extras {namespace cmex {

	/// simple wrapper around mxArray which make the contained mxArray* persistent
	class persistentMxArray {
		mxArray* _mxptr = nullptr; //pointer to managed mxArray
	public:

		/// destroy mxarray
		~persistentMxArray() {
			if (_mxptr != nullptr) {
				mxDestroyArray(_mxptr);
			}
		}

		/// create uninitialized mxArray wrapper
		persistentMxArray() {};

		/// create persistentMxarray by copying source mxArray
		persistentMxArray(const mxArray* src) {
			_mxptr = mxDuplicateArray(src);
			mexMakeArrayPersistent(_mxptr);
		}

		// create by copy
		persistentMxArray(const persistentMxArray& src) {
			_mxptr = mxDuplicateArray(src._mxptr);
			mexMakeArrayPersistent(_mxptr);
		}

		/// create by move
		persistentMxArray(persistentMxArray&& src) {
			_mxptr = src._mxptr;
			src._mxptr = nullptr;
		}

		/// set by copying source mxArray
		persistentMxArray& operator=(const mxArray* src) {
			_mxptr = mxDuplicateArray(src);
			mexMakeArrayPersistent(_mxptr);
			return *this;
		}

		/// set by copy
		persistentMxArray& operator=(const persistentMxArray& src) {
			_mxptr = mxDuplicateArray(src._mxptr);
			mexMakeArrayPersistent(_mxptr);
			return *this;
		}

		/// set by move
		persistentMxArray& operator=(persistentMxArray&& src) {
			_mxptr = src._mxptr;
			src._mxptr = nullptr;
			return *this;
		}

		/// retun const pointer to mxArray
		operator const mxArray*() const {
			return _mxptr;
		}

		/// return a non-persistent copy of mxarray
		mxArray* getMxArray() const {
			if (_mxptr == nullptr) {
				return nullptr;
			}
			return mxDuplicateArray(_mxptr);
		}

	};

}}