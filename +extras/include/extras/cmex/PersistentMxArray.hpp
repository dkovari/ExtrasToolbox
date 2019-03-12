#pragma once
/* PersistentMxArray.hpp
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
*/

#include <mex.h>
#include <mutex>

namespace extras {namespace cmex {

	/// simple wrapper around mxArray which make the contained mxArray* persistent
	class persistentMxArray {
		mutable std::mutex _mxptrMutex;
		mxArray* _mxptr = nullptr; //pointer to managed mxArray
	public:

		/// destroy mxarray
		~persistentMxArray() {
			if (_mxptr != nullptr) {
				std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
				mxDestroyArray(_mxptr);
			}
		}

		/// create uninitialized mxArray wrapper
		persistentMxArray() {};

		/// create persistentMxarray by copying source mxArray
		persistentMxArray(const mxArray* src) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			_mxptr = mxDuplicateArray(src);
			mexMakeArrayPersistent(_mxptr);
		}

		/// create by copy
		persistentMxArray(const persistentMxArray& src) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			std::lock_guard<std::mutex> lock_src(src._mxptrMutex); //lock _mxptr;
			_mxptr = mxDuplicateArray(src._mxptr);
			mexMakeArrayPersistent(_mxptr);
		}

		/// create by move
		persistentMxArray(persistentMxArray&& src) {
			std::lock_guard<std::mutex> lock_src(src._mxptrMutex); //lock _mxptr;
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			_mxptr = src._mxptr;
			src._mxptr = nullptr;
		}

		/// set by copying source mxArray
		persistentMxArray& operator=(const mxArray* src) {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			_mxptr = mxDuplicateArray(src);
			mexMakeArrayPersistent(_mxptr);
			return *this;
		}

		/// set by copy
		persistentMxArray& operator=(const persistentMxArray& src) {
			std::lock_guard<std::mutex> lock_src(src._mxptrMutex); //lock _mxptr;
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			_mxptr = mxDuplicateArray(src._mxptr);
			mexMakeArrayPersistent(_mxptr);
			return *this;
		}

		/// set by move
		persistentMxArray& operator=(persistentMxArray&& src) {
			std::lock_guard<std::mutex> lock_src(src._mxptrMutex); //lock _mxptr;
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			_mxptr = src._mxptr;
			src._mxptr = nullptr;
			return *this;
		}

		/// return const pointer to mxArray
		operator const mxArray*() const {
			return _mxptr;
		}

		/// return a non-persistent copy of mxarray
		mxArray* getMxArray() const {
			if (_mxptr == nullptr) {
				return nullptr;
			}
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			return mxDuplicateArray(_mxptr);
		}

		/// return non-const, persistent mxArray*
		/// NOTE: the returned array will be managed by persistentMxArray()
		//	DO NOT DELETE THE ARRAY!
		mxArray* getPersistentArray() {
			return _mxptr;
		}

		/// return const, persistent mxArray*
		const mxArray* getConstArray() const{
			return _mxptr;
		}

	};

}}