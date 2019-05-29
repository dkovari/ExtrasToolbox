/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

/*
Wrapper Class from which MxStruct and MxClass Wrappers are derived
*/

#include "mxobject.hpp"

namespace extras {namespace cmex {

	/** Virtual Wrapper Class from which MxStruct and MxClass Wrappers are derived
	*/
	class MxWrapper {
	protected:
		virtual void internalSet(mxArray* src) = 0;
		virtual mxArray* internalGet() = 0;
		virtual const mxArray* internalGet() const = 0;
	public:

		
		//! returns const mxArray*()
		operator const mxArray*() const { return internalGet(); }

		operator mxArray*() { return internalGet(); }

		//! returns (non-const) mxArray* _mxptr
		// does not change memory management rules
		mxArray* getmxarray() { return internalGet(); }

		//! return const mxArray*
		const mxArray* getConstArray() const {
			return internalGet();
		}

		//! move mxArray* into field
		//! pvalue will be managed by the struct after operation
		MxWrapper& operator=(mxArray* pvalue) {
			internalSet(pvalue);
			return *this;
		}

		//! set field from const array
		//! duplicated array
		MxWrapper& operator=(const mxArray* pvalue) {
			internalSet(mxDuplicateArray(pvalue));
			return *this;
		}

		//! set field equal to string
		MxWrapper& operator=(const char* str) {
			internalSet(mxCreateString(str));
			return *this;
		}

		//! set field equal to scalar double
		MxWrapper& operator=(double val) {
			internalSet(mxCreateDoubleScalar(val));
			return *this;
		}

		//! Move from MxObject
		MxWrapper& operator=(MxObject&& src) {
			internalSet(src);
			return *this;
		}

		////////////////////////////
		// Type Conversions

		operator double() const {
			const mxArray* pA = internalGet();
			if (!mxIsScalar(pA)) {
				throw(extras::stacktrace_error(
					std::string("MxWrapper::double(): cannot cast non-scalar to double.\nType: ")
					+ mxGetClassName(pA)
					+ std::string("\nnumel:")
					+ std::to_string(mxGetNumberOfElements(pA))));
			}

			if (!mxIsNumeric(pA) && !mxIsLogical(pA)) {
				throw(extras::stacktrace_error(
					std::string("MxWrapper::double(): cannot cast non-numeric to double.\nType: ")
					+ mxGetClassName(pA)
					+ std::string("\nnumel:")
					+ std::to_string(mxGetNumberOfElements(pA))));
			}
			return mxGetScalar(pA);
		}
		operator std::string() const {
			return getstring(internalGet());
		}

		/** Cast to bool
		*/
		/* NOT WORKING
		operator bool() const {
			const mxArray* pA = internalGet();
			if (!mxIsScalar(pA)) {
				throw(extras::stacktrace_error(
					std::string("MxWrapper::bool(): cannot cast non-scalar to bool.\nType: ")
					+ mxGetClassName(pA)
					+ std::string(" numel:")
					+ std::to_string(mxGetNumberOfElements(pA))));
			}
			if (!mxIsNumeric(pA) && !mxIsLogical(pA)) {
				throw(extras::stacktrace_error(
					std::string("MxWrapper::bool(): cannot cast non-numeric or non-logical to bool.\nType: ")
					+ mxGetClassName(pA)
					+ std::string(" numel:")
					+ std::to_string(mxGetNumberOfElements(pA))));
			}
			double val = mxGetScalar(pA);
			return (val != (double)0.0);
		}*/

		////////////////////
		// Array Info

		bool isempty() const { return mxIsEmpty(internalGet()); }
		bool isscalar() const { return mxIsScalar(internalGet()); }
		bool isnumeric() const { return mxIsNumeric(internalGet()); }
		bool isstruct() const { return mxIsStruct(internalGet()); }
		bool ischar() const { return mxIsChar(internalGet()); }
		bool iscell() const { return mxIsCell(internalGet()); }
		bool isfinite() const { return mxIsFinite(mxGetScalar(internalGet())); }
		bool isinf() const { return mxIsInf(mxGetScalar(internalGet())); }
		bool islogical() const { return mxIsLogical(internalGet()); }

		size_t numel() const { return mxGetNumberOfElements(internalGet()); }
		size_t ndims() const { return mxGetNumberOfDimensions(internalGet()); }

		std::vector<size_t> size() const {
			return cmex::size(internalGet());
		}

	};

}}