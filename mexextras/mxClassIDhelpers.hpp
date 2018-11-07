#pragma once

#include <mex.h>
#include <typeinfo>
#include <cstdint>


/// Return element size from mxclassID
size_t elementsize(mxClassID ClassID);

/// return true if mxClassID is numeric type
bool isnumeric(mxClassID ClassID);

/// convert mxClassID to char array
const char* classname(mxClassID ClassID);

/// return true if mxClassID is integer type
bool isint(mxClassID ClassID);


/////////////////////////////
// Implementation Below

size_t elementsize(mxClassID ClassID){
	switch (ClassID) {
	case mxDOUBLE_CLASS:
		return sizeof(double);
	case mxSINGLE_CLASS:
		return sizeof(float);
	case mxINT8_CLASS:
		return sizeof(int8_t);
	case mxUINT8_CLASS:
		return sizeof(uint8_t);
	case mxINT16_CLASS:
		return sizeof(int16_t);
	case mxUINT16_CLASS:
		return sizeof(uint16_t);
	case mxINT32_CLASS:
		return sizeof(int32_t);
	case mxUINT32_CLASS:
		return sizeof(uint32_t);
		break;
	case mxINT64_CLASS:
		return sizeof(int64_t);
	case mxUINT64_CLASS:
		return sizeof(uint64_t);
	default:
		return 0;
	}
}

bool isnumeric(mxClassID ClassID) {
	switch (ClassID) {
	case mxDOUBLE_CLASS:
		return true;
	case mxSINGLE_CLASS:
		return true;
	case mxINT8_CLASS:
		return true;
	case mxUINT8_CLASS:
		return true;
	case mxINT16_CLASS:
		return true;
	case mxUINT16_CLASS:
		return true;
	case mxINT32_CLASS:
		return true;
	case mxUINT32_CLASS:
		return true;
	case mxINT64_CLASS:
		return true;
	case mxUINT64_CLASS:
		return true;
	default:
		return false;
	}
}

bool isnumeric(const mxArray* mxa){
	return isnumeric(mxGetClassID(mxa));
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
