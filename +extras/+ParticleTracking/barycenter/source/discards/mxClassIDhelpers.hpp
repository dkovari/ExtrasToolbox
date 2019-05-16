#pragma once

#include <mex.h>
#include <typeinfo>
#include <cstdint>


size_t elementsize(mxClassID ClassID);

bool isnumeric(mxClassID ClassID);

const char* classname(mxClassID ClassID);

bool isint(mxClassID ClassID);

bool sametype(const std::type_info&, const mxClassID &);

mxClassID type2ClassID(const std::type_info& T1);


size_t elementsize(mxClassID ClassID)
{
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

bool sametype(const std::type_info& T1, const mxClassID & cID) {
	switch (cID) {
	case mxDOUBLE_CLASS:
		return T1 == typeid(double);
	case mxSINGLE_CLASS:
		return T1 == typeid(float);
	case mxINT8_CLASS:
		return T1 == typeid(int8_t);
	case mxUINT8_CLASS:
		return T1 == typeid(uint8_t);
	case mxINT16_CLASS:
		return T1 == typeid(int16_t);
	case mxUINT16_CLASS:
		return T1 == typeid(uint16_t);
	case mxINT32_CLASS:
		return T1 == typeid(int32_t);
	case mxUINT32_CLASS:
		return T1 == typeid(uint32_t);
		break;
	case mxINT64_CLASS:
		return T1 == typeid(int64_t);
	case mxUINT64_CLASS:
		return T1 == typeid(uint64_t);
	default:
		return false;
	}
}

mxClassID type2ClassID(const std::type_info& T1) {
	if (T1 == typeid(double)) {
		return mxDOUBLE_CLASS;
	}
	if (T1 == typeid(float)) {
		return mxSINGLE_CLASS;
	}
	if (T1 == typeid(int8_t)) {
		return mxINT8_CLASS;
	}
	if (T1 == typeid(uint8_t)) {
		return mxUINT8_CLASS;
	}
	if (T1 == typeid(int16_t)) {
		return mxINT16_CLASS;
	}
	if (T1 == typeid(uint16_t)) {
		return mxUINT16_CLASS;
	}
	if (T1 == typeid(int32_t)) {
		return mxINT32_CLASS;
	}
	if (T1 == typeid(uint32_t)) {
		return mxUINT32_CLASS;
	}
	if (T1 == typeid(int64_t)) {
		return mxINT64_CLASS;
	}
	if (T1 == typeid(uint64_t)) {
		return mxUINT64_CLASS;
	}
	if(T1==typeid(bool)){
		return mxLOGICAL_CLASS;
	}
	return mxUNKNOWN_CLASS;
}
