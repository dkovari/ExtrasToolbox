/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <mex.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace extras{namespace cmex{

    //! group of thread-safe mxArray pointers
    //! useful for capturing all of the arguments passed to a mex function and storing them in a list or queue
	class mxArrayGroup {
	private:
		std::vector<mxArray*> _mxptrs; //vector of mxarray*

		void destroyArrays() {
			for (auto p : _mxptrs) {
				mxDestroyArray(p);
			}
			_mxptrs.clear();
		}

		void copyFrom(const mxArrayGroup& src) {
			destroyArrays();
			_mxptrs.resize(src.size(), nullptr);
			for (size_t n = 0; n < src.size(); n++) {
				_mxptrs[n] = mxDuplicateArray(src[n]);
				mexMakeArrayPersistent(_mxptrs[n]);
			}
		}

		void moveFrom(mxArrayGroup& src) {
			destroyArrays();
			_mxptrs = std::move(src._mxptrs);
			src._mxptrs.clear();
		}

	public:

		///////////
		// destructor
		virtual ~mxArrayGroup() {
			destroyArrays();
		}

		//////////////////////////
		// Constructors

		//! default constructor
		mxArrayGroup() {

		}

		//! construct with certain size
		//! array pointers will be nullptr
		mxArrayGroup(size_t n) {
			_mxptrs.resize(n, nullptr);
		}

		//! construct by taking ownership of array of mxarray*
		mxArrayGroup(size_t n, mxArray** pArrays) {
			moveFrom(n, pArrays);
		}

		//! construct by making copy of array of const mxarray*
		mxArrayGroup(size_t n, const mxArray* prhs[]) {
			copyFrom(n, prhs);
		}

		//! Copy Construction
		mxArrayGroup(const mxArrayGroup& src) {
			copyFrom(src);
		}

		//! copy assignment
		mxArrayGroup& operator=(const mxArrayGroup& src) {
			copyFrom(src);
			return *this;
		}

		//! move Construction
		mxArrayGroup(mxArrayGroup&& src) {
			moveFrom(src);
		}

		//! move assignment
		mxArrayGroup& operator=(mxArrayGroup&& src) {
			moveFrom(src);
			return *this;
		}

		//! Copy arrays to destination array or mxarray*
		//! the copied arrays will not be managed by the mxarraygroup
		//! so it is YOUR job to delete them when you're done
		//! if nlhs > size() then the extra mxArray* will be nullptr
		//! Resulting mxarray* will not be persistent
		void copyTo(size_t nlhs, mxArray** plhs) {
			for (size_t n = 0; n < nlhs; n++) {
				if (n >= _mxptrs.size()) {
					plhs[n] = nullptr;
				}
				else {
					plhs[n] = mxDuplicateArray(_mxptrs[n]);
				}
			}
		}

		//! Take ownership of array of mxarray*
		//! after calling, the arrays will be managed by mxArraygroup
		void moveFrom(size_t nrhs, mxArray** prhs) {
			destroyArrays();
			_mxptrs.resize(nrhs, nullptr);
			for (size_t i = 0; i < nrhs; i++) {
				mexMakeArrayPersistent(prhs[i]);
				_mxptrs[i] = prhs[i];
			}
		}

		//! Create a copy of the arrays
		void copyFrom(size_t nrhs, const mxArray* prhs[]) {
			destroyArrays();
			_mxptrs.resize(nrhs, nullptr);
			for (size_t i = 0; i < nrhs; i++) {
				_mxptrs[i] = mxDuplicateArray(prhs[i]);
				mexMakeArrayPersistent(_mxptrs[i]);
			}
		}

		//! push (non-const) mxArray* to back
		//! will take ownership of array;
		void push_back(mxArray* pA) {
			mexMakeArrayPersistent(pA);
			_mxptrs.push_back(pA);
		}

		//! push const mxArray* to back
		//! makes copy of array
		void push_back(const mxArray* pA) {
			mxArray* p = mxDuplicateArray(pA);
			mexMakeArrayPersistent(p);
			_mxptrs.push_back(p);
		}

		void resize(size_t n) {
			if (n < _mxptrs.size()) { //clear extra ptrs
				for (size_t i = n; i < _mxptrs.size(); i++) {
					mxDestroyArray(_mxptrs[i]);
				}
			}
			_mxptrs.resize(n, nullptr);
		}

		void clear() {
			destroyArrays();
		}

		///////////////////////
		// Info methods

		//! return number of contained arrays
		size_t size() const { return _mxptrs.size(); }

		/////////////////////////
		// Element methods

		//! get array at index n
		//! does not release array from mxarraygroup control
		//! so DO NOT DELETE, also ptr will not live past the mxarraygroup
		mxArray* operator[](size_t n) {
			return _mxptrs[n];
		}

		//! get const mxarray* at index n
		const mxArray* operator[](size_t n) const{
			return _mxptrs[n];
		}

		//! get array at index n
		//! does not release array from mxarraygroup control
		//! so DO NOT DELETE, also ptr will not live past the mxarraygroup
		mxArray* getArray(size_t n){
			return _mxptrs[n];
		}

		//! get array at index n
		//! does not release array from mxarraygroup control
		const mxArray* getConstArray(size_t n) const {
			return _mxptrs[n];
		}

		//! copy mxArray* to index i
		void setArray(size_t i, const mxArray* psrc) {
			if (i >= size()) {
				_mxptrs.resize(i + 1, nullptr);
			}
			_mxptrs[i] = mxDuplicateArray(psrc);
			mexMakeArrayPersistent(_mxptrs[i]);
		}

		//! move mxArray* to index i
		void ownArray(size_t i, mxArray* psrc) {
			if (i >= size()) {
				_mxptrs.resize(i + 1, nullptr);
			}
			_mxptrs[i] = psrc;
			mexMakeArrayPersistent(_mxptrs[i]);
		}

		//! get array of  const mxArray*
		operator const mxArray **() const{
			return (const mxArray **)_mxptrs.data();
		}

		//////////////////
		// Iterators

		typedef std::vector<mxArray*>::iterator iterator;
		typedef std::vector<mxArray*>::const_iterator const_iterator;
		iterator begin() { return _mxptrs.begin(); }
		iterator end() { return _mxptrs.end(); }
		const_iterator begin() const { return _mxptrs.begin(); }
		const_iterator end() const { return _mxptrs.end(); }
	};
}}
