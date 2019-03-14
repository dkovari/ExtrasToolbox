/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "mxobject.hpp"

namespace extras{namespace cmex{
	
	class MxStruct;
	class FieldWrapper;

    //! Extension of MxObject for struct array support
    //! fields can be accessed using a simple syntax:
    //!     StructArray(index,"FieldName") = ...//pointer to mxArray to set
    //!     const mxArray* field = StructArray(index,"FieldName"); //get pointer to field
    class MxStruct:public MxObject{
		friend class FieldWrapper;
    public:
        ////////////////////////
        // Constructors

		//! Construct MxStruct of size [1,1] with no fields
        MxStruct():MxObject(){
			own(mxCreateStructMatrix(1, 1, 0, nullptr));
        };

		//! Construct MxStruct of size [numel,1] with no fields
		MxStruct(size_t numel) :MxObject() {
			own(mxCreateStructMatrix(numel, 1, 0, nullptr));
		}

		//! Construct MxStruct of size [numel,1] with fields specified by vector of strings
		MxStruct(size_t numel, const std::vector<std::string>& fieldnames) :MxObject() {

			const char** names = new const char*[fieldnames.size()];
			
			size_t k = 0;
			for (auto& nm : fieldnames) {
				names[k] = nm.c_str();
				k++;
			}

			own(mxCreateStructMatrix(numel, 1, fieldnames.size(), names));
			delete[] names;
		}

		/*/! Construct MxStruct of size [numel,1] with fields specified by list of strings
		MxStruct(size_t numel, const std::list<std::string>& fieldnames) :MxObject() {

			const char** names = new const char*[fieldnames.size()];

			size_t k = 0;
			for (auto& nm : fieldnames) {
				names[k] = nm.c_str();
				k++;
			}

			own(mxCreateStructMatrix(numel, 1, fieldnames.size(), names));
			delete[] names;
		}*/

		//! Construct MxStruct of size specified by dims with fields specified by vector of strings
		MxStruct(const std::vector<size_t>& dims, const std::vector<std::string>& fieldnames) :MxObject() {

			const char** names = new const char*[fieldnames.size()];

			size_t k = 0;
			for (auto& nm : fieldnames) {
				names[k] = nm.c_str();
				k++;
			}
			own(mxCreateStructArray(dims.size(), dims.data(), fieldnames.size(), names));
			delete[] names;
		}

		/*/! Construct MxStruct of size specified by dims with fields specified by list of strings
		MxStruct(const std::vector<size_t>& dims, const std::list<std::string>& fieldnames) :MxObject() {

			const char** names = new const char*[fieldnames.size()];

			size_t k = 0;
			for (auto& nm : fieldnames) {
				names[k] = nm.c_str();
				k++;
			}
			own(mxCreateStructArray(dims.size(), dims.data(), fieldnames.size(), names));
			delete[] names;
		}*/

		//! copy construction
        MxStruct(const MxObject& src): MxObject(src){
            if(!mxIsStruct(_mxptr)){
                throw(std::runtime_error("MxStruct(const MxObject& src): src is not a struct."));
            }
        }
       
		//! move construction
		MxStruct(MxObject&& src): MxObject(std::move(src)){
            if(!mxIsStruct(_mxptr)){
                throw(std::runtime_error("MxStruct(MxObject&& src): src is not a struct."));
            }
        }

        //! copy assignment
        MxStruct& operator=(const MxObject& src){
            if(!src.isstruct()){
                throw(std::runtime_error("MxStruct::operator=(const MxObject& src): src is not a struct."));
            }
			copyFrom(src);
            return *this;
        }

        //! move assignment
        MxStruct& operator=(MxObject && src){
            if(!src.isstruct()){
                throw(std::runtime_error("MxStruct::operator=(MxObject && src): src is not a struct."));
            }
			moveFrom(src);
            return *this;
        }

        //! Construct and assign from mxarray
        //! assume mxArray* is not persistent
        MxStruct(mxArray* mxptr,bool isPersistent=false):
            MxObject(mxptr,isPersistent)
        {
            //mexPrintf("mxObject(mx*):%d fromL %d\n",this,mxptr);
            if(!mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxStruct& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            if(!mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }
        MxStruct(const mxArray* mxptr, bool isPersistent = false):
            MxObject(mxptr,isPersistent)
        {
            if(!mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxStruct& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            if(!mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }

        ////////////////////////////////////////
        // Field Access

		//! returns true if fieldname is a valid field in the struct
		bool isfield(const char* fieldname) {
			return mxGetFieldNumber(_mxptr, fieldname) >= 0;
		}

        //! non-const access to field
		//! returns FieldWrapper
		//! NOTE: FieldWrappers may not be thread-safe
		//! if the parent MxStruct is changed by a different thread, the field wrapper will probably be corrupted
		FieldWrapper operator()(size_t idx, const char* fieldname);

        //! const access to field
        const mxArray* operator()(size_t idx, const char* fieldname) const{
            if(idx>=mxGetNumberOfElements(_mxptr)){
                throw(std::runtime_error("MxStruct::operator() index exceeds struct array dimension"));
            }
            return mxGetField(_mxptr,idx,fieldname);
        }

		//! return number of fields
		//! does not lock internal mutex
		size_t number_of_fields() const {
			return mxGetNumberOfFields(_mxptr);
		}

		//! return the fieldnames of the struct
		//! locks internal mutex
		std::vector<std::string> fieldnames() const {
			std::lock_guard<std::mutex> lock(_mxptrMutex); //lock _mxptr;
			size_t nfields = number_of_fields();
			std::vector<std::string> out;
			out.reserve(nfields);

			for (size_t k = 0; k < nfields; ++k) {
				out.push_back(mxGetFieldNameByNumber(_mxptr, k));
			}
			return out;
		}

    };

	
	//! Wrapper around mxArray struct field elements
	//! provides simple get and set support using operator=
	class FieldWrapper {
		friend class MxStruct;
	protected:
		MxStruct& _parent;
		//mxArray* parent=nullptr;
		int field_number = -1;
		size_t index = 0;

		//! Construct FieldWrapper (not copy/move)
		//! this should only be called by MxStruct
		FieldWrapper(MxStruct& parent, size_t idx, const char* fieldname) :
			_parent(parent),
			index(idx)
		{
			field_number = mxGetFieldNumber(_parent._mxptr, fieldname);
			if (field_number<0) { //need to create field

				if (_parent._setFromConst) {
					throw(std::runtime_error("FieldWrapper: Cannot add field to const mxArray*"));
				}

				field_number = mxAddField(_parent._mxptr, fieldname);
			}

			if (field_number<0) {
				throw(std::runtime_error(std::string("FieldWrapper(): Could not create fieldname:") + std::string(fieldname)));
			}
		}

		//! Construct FieldWrapper using field index only
		FieldWrapper(MxStruct& parent, size_t idx, int fieldnumber) :
			_parent(parent),
			field_number(fieldnumber),
			index(idx)
		{
			if (field_number<0) {
				throw(std::runtime_error("invalid field number"));
			}
		}

		/* BROKEN NOW THAT WE USE MxStruct& as parent
		FieldWrapper(const mxArray* parentPtr, size_t idx, const char* fieldname) :
			parent((mxArray*)parentPtr),
			index(idx),
			_setFromConst(true)
		{
			field_number = mxGetFieldNumber(parent, fieldname);
			if (field_number<0) { //need to create field
				if (_setFromConst) {
					throw(std::runtime_error("FieldWrapper: Cannot add field to const mxArray*"));
				}
				field_number = mxAddField(parent, fieldname);
			}

			if (field_number<0) {
				throw(std::runtime_error(std::string("FieldWrapper(): Could not create fieldname:") + std::string(fieldname)));
			}
		}

		FieldWrapper(const mxArray* parentPtr, size_t idx, int fieldnumber) :
			parent((mxArray*)parentPtr),
			field_number(fieldnumber),
			index(idx),
			_setFromConst(true)
		{
			if (field_number<0) {
				throw(std::runtime_error("invalid field number"));
			}
		}
		*/

	public:
		FieldWrapper(const FieldWrapper& src) = default;
		FieldWrapper(FieldWrapper&& src) = default;
		FieldWrapper& operator=(const FieldWrapper& src) = default;
		FieldWrapper& operator=(FieldWrapper&& src) = default;


		//! get field
		//! returns const mxArray*()
		operator const mxArray*() const {
			return mxGetFieldByNumber(_parent._mxptr, index, field_number);
		}

		//! set field
		FieldWrapper& operator=(mxArray* pvalue) {

			if (_parent._setFromConst) {
				throw(std::runtime_error("FieldWrapper: Cannot use operator= for FieldWrapper created from const mxArray*"));
			}


			mxDestroyArray(mxGetFieldByNumber(_parent._mxptr, index, field_number));
			//mxSetFieldByNumber(parent, index, field_number, nullptr);
			//mexPrintf("pvalue: %p\n", pvalue);
			mxSetFieldByNumber(_parent._mxptr, index, field_number, pvalue);
			//mexPrintf("after set: %p\n", mxGetFieldByNumber(parent, index, field_number));
			return *this;
		}

		//! set field from const array
		//! duplicated array
		FieldWrapper& operator=(const mxArray* pvalue) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("FieldWrapper: Cannot use operator= for FieldWrapper created from const mxArray*"));
			}


			mxDestroyArray(mxGetFieldByNumber(_parent._mxptr, index, field_number));
			mxSetFieldByNumber(_parent._mxptr, index, field_number, mxDuplicateArray(pvalue));
			return *this;
		}

		//! set field equal to string
		FieldWrapper& operator=(const char* str) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("FieldWrapper: Cannot use operator= for FieldWrapper created from const mxArray*"));
			}


			mxDestroyArray(mxGetFieldByNumber(_parent._mxptr, index, field_number));
			mxSetFieldByNumber(_parent._mxptr, index, field_number, mxCreateString(str));
			return *this;
		}

		//! set field equal to scalar double
		FieldWrapper& operator=(double val) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("FieldWrapper: Cannot use operator= for FieldWrapper created from const mxArray*"));
			}


			mxDestroyArray(mxGetFieldByNumber(_parent._mxptr, index, field_number));
			mxSetFieldByNumber(_parent._mxptr, index, field_number, mxCreateDoubleScalar(val));
			return *this;
		}

		//! Move from MxObject
		FieldWrapper& operator=(MxObject&& src) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("FieldWrapper: Cannot use operator= for FieldWrapper created from const mxArray*"));
			}


			mxDestroyArray(mxGetFieldByNumber(_parent._mxptr, index, field_number));
			mxSetFieldByNumber(_parent._mxptr, index, field_number, src);
			return *this;

		}

	};

	////////////////////////
	// Method Defs needing FieldWrapper

	//! non-const access to field
	//! returns FieldWrapper
	//! NOTE: FieldWrappers may not be thread-safe
	//! if the parent MxStruct is changed by a different thread, the field wrapper will probably be corrupted
	FieldWrapper MxStruct::operator()(size_t idx, const char* fieldname) {
		if (idx >= mxGetNumberOfElements(_mxptr)) {
			throw(std::runtime_error("MxStruct::operator() index exceeds struct array dimension"));
		}
		return FieldWrapper(*this, idx, fieldname);
	}


}}
