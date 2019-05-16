/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <mex.h>
#include <typeinfo>
#include <cstdint>


namespace extras{namespace cmex{


//! returns number of bytes used by data element for give type
size_t elementBytes(mxClassID cid, bool isComplex = false);

//! return true if mxClassID is numeric type
bool isnumeric(mxClassID ClassID);

//! convert mxClassID to char array
const char* classname(mxClassID ClassID);

//! return true if mxClassID is integer type
bool isint(mxClassID ClassID);


/////////////////////////////
// Implementation Below

//! returns number of bytes used by data element for give type
size_t elementBytes(mxClassID cid, bool isComplex) {
	switch (cid) {
	case mxLOGICAL_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(bool);
		}
		else {
			return sizeof(bool);
		}
#else
		return sizeof(double);
#endif
	case mxDOUBLE_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(double);
		}
		else {
			return sizeof(double);
		}
#else
		return sizeof(double);
#endif
	case mxSINGLE_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(float);
		}
		else {
			return sizeof(float);
		}
#else
		return sizeof(float);
#endif
	case mxINT8_CLASS:
	case mxUINT8_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(int8_t);
		}
		else {
			return sizeof(int8_t);
		}
#else
		return sizeof(int8_t);
#endif
	case mxINT16_CLASS:
	case mxUINT16_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(int16_t);
		}
		else {
			return sizeof(int16_t);
		}
#else
		return sizeof(int16_t);
#endif
	case mxINT32_CLASS:
	case mxUINT32_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(int32_t);
		}
		else {
			return sizeof(int32_t);
		}
#else
		return sizeof(int32_t);
#endif
	case mxINT64_CLASS:
	case mxUINT64_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(int64_t);
		}
		else {
			return sizeof(int64_t);
		}
#else
		return sizeof(int64_t);
#endif
	case mxCHAR_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * 2;
		}
		else {
			return 2;
		}
#else
		return 2;
#endif
	case mxCELL_CLASS:
	case mxSTRUCT_CLASS:
#if MX_HAS_INTERLEAVED_COMPLEX
		if (isComplex) {
			return 2 * sizeof(void*);
		}
		else {
			return sizeof(void*);
		}
#else
		return sizeof(void*);
#endif
	default:
		throw(extras::stacktrace_error(std::string("elementByte(): not implemented for mxClassID=") + std::string(classname(cid))));
	}
}

bool isnumeric(mxClassID ClassID) {
	switch (ClassID) {
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
		return true;
	default:
		return false;
	}
}

const char* classname(mxClassID ClassID) {
	switch (ClassID) {
	case mxDOUBLE_CLASS:
		return "double";
	case mxSINGLE_CLASS:
		return "single";
	case mxINT8_CLASS:
		return "int8";
	case mxUINT8_CLASS:
		return "uint8";
	case mxINT16_CLASS:
		return "int16";
	case mxUINT16_CLASS:
		return "uint16";
	case mxINT32_CLASS:
		return "int32";
	case mxUINT32_CLASS:
		return "uint32";
	case mxINT64_CLASS:
		return "int64";
	case mxUINT64_CLASS:
		return "uint64";
	case mxCELL_CLASS:
		return "cell";
	case mxFUNCTION_CLASS:
		return "function_handle";
	case mxLOGICAL_CLASS:
		return "logical";
	case mxCHAR_CLASS:
		return "char";
	case mxSTRUCT_CLASS:
		return "struct";
	default:
		return "unknown";
	}
}

bool isint(mxClassID ClassID) {
	if (!isnumeric(ClassID))
		return false;

	if (ClassID == mxDOUBLE_CLASS || ClassID == mxSINGLE_CLASS)
		return false;

	return true;
}

}}
