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

		//! copy construction
        MxStruct(const MxObject& src): MxObject(src){
            if(!isstruct()){
                throw(std::runtime_error("MxStruct(const MxObject& src): src is not a struct."));
            }
        }
       
		//! move construction
		MxStruct(MxObject&& src): MxObject(std::move(src)){
            if(!isstruct()){
                throw(extras::stacktrace_error("MxStruct(MxObject&& src): src is not a struct."));
            }
        }

        //! copy assignment
        MxStruct& operator=(const MxObject& src){
            if(!src.isstruct()){
                throw(extras::stacktrace_error("MxStruct::operator=(const MxObject& src): src is not a struct."));
            }
			copyFrom(src);
            return *this;
        }

        //! move assignment
        MxStruct& operator=(MxObject && src){
            if(!src.isstruct()){
                throw(extras::stacktrace_error("MxStruct::operator=(MxObject && src): src is not a struct."));
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
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxStruct& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            if(!mxIsStruct(mxptr)){
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }
        MxStruct(const mxArray* mxptr, bool isPersistent = false):
            MxObject(mxptr,isPersistent)
        {
            if(!mxIsStruct(mxptr)){
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxStruct& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            if(!mxIsStruct(mxptr)){
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }

        ////////////////////////////////////////
        // Field Access

		//! returns true if fieldname is a valid field in the struct
		bool isfield(const char* fieldname) {
			return mxGetFieldNumber(getmxarray(), fieldname) >= 0;
		}

        //! non-const access to field
		//! returns FieldWrapper
		//! NOTE: FieldWrappers may not be thread-safe
		//! if the parent MxStruct is changed by a different thread, the field wrapper will probably be corrupted
		FieldWrapper operator()(size_t idx, const char* fieldname);

        //! const access to field
        const mxArray* operator()(size_t idx, const char* fieldname) const{
            if(idx>=numel()){
                throw(extras::stacktrace_error("MxStruct::operator() index exceeds struct array dimension"));
            }
            return mxGetField(getmxarray(),idx,fieldname);
        }

		//! return number of fields
		//! does not lock internal mutex
		size_t number_of_fields() const {
			return mxGetNumberOfFields(getmxarray());
		}

		//! return the fieldnames of the struct
		//! locks internal mutex
		std::vector<std::string> fieldnames() const {
			size_t nfields = number_of_fields();
			std::vector<std::string> out;
			out.reserve(nfields);

			for (size_t k = 0; k < nfields; ++k) {
				out.push_back(mxGetFieldNameByNumber(getmxarray(), k));
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
			field_number = mxGetFieldNumber(_parent.getmxarray(), fieldname);
			if (field_number<0) { //need to create field

				if (_parent.isConst()) {
					throw(extras::stacktrace_error("FieldWrapper: Cannot add field to const mxArray*"));
				}

				field_number = mxAddField(_parent.getmxarray(), fieldname);
			}

			if (field_number<0) {
				throw(extras::stacktrace_error(std::string("FieldWrapper(): Could not create fieldname:") + std::string(fieldname)));
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

		//! set the linked field
		//! NOTE: src will be managed by the struct after this, so don't try to delete or rely on src after calling internalSet()
		void internalSet(mxArray* src) {
			if (_parent.isConst()) {
				throw(extras::stacktrace_error("FieldWrapper: Cannot use operator= for FieldWrapper created from const mxArray*"));
			}
			mxDestroyArray(mxGetFieldByNumber(_parent.getmxarray(), index, field_number));
			mxSetFieldByNumber(_parent.getmxarray(), index, field_number, src);
		}

	public:
		FieldWrapper(const FieldWrapper& src) = default;
		FieldWrapper(FieldWrapper&& src) = default;
		FieldWrapper& operator=(const FieldWrapper& src) = default;
		FieldWrapper& operator=(FieldWrapper&& src) = default;


		//! get field
		//! returns const mxArray*()
		operator const mxArray*() const {
			return mxGetFieldByNumber(_parent.getmxarray(), index, field_number);
		}

		//! move mxArray* into field
		//! pvalue will be managed by the struct after operation
		FieldWrapper& operator=(mxArray* pvalue) {
			internalSet(pvalue);
			return *this;
		}

		//! set field from const array
		//! duplicated array
		FieldWrapper& operator=(const mxArray* pvalue) {
			internalSet(mxDuplicateArray(pvalue));
			return *this;
		}

		//! set field equal to string
		FieldWrapper& operator=(const char* str) {
			internalSet(mxCreateString(str));
			return *this;
		}

		//! set field equal to scalar double
		FieldWrapper& operator=(double val) {
			internalSet(mxCreateDoubleScalar(val));
			return *this;
		}

		//! Move from MxObject
		FieldWrapper& operator=(MxObject&& src) {
			internalSet(src);
			return *this;
		}

		////////////////////////////
		// Type Conversions

		operator double() const {
			if (!_parent.isnumeric() || !_parent.isscalar()) {
				throw(extras::stacktrace_error("FieldWrapper::double(): cannot cast non-numeric or non-scalar to double"));
			}
			return mxGetScalar(_parent.getmxarray());
		}

		operator std::string() const {
			return getstring(_parent.getmxarray());
		}

		bool isempty() const {
			return _parent.isempty();
		}

		bool isscalar() const {
			return _parent.isscalar();
		}

		bool iscell() const {
			return _parent.iscell();
		}

		bool isnumeric() const {
			return _parent.isnumeric();
		}

		bool isstruct() const {
			return _parent.isstruct();
		}

		bool ischar() const {
			return _parent.ischar();
		}

	};

	////////////////////////
	// Method Defs needing FieldWrapper

	//! non-const access to field
	//! returns FieldWrapper
	//! NOTE: FieldWrappers may not be thread-safe
	//! if the parent MxStruct is changed by a different thread, the field wrapper will probably be corrupted
	FieldWrapper MxStruct::operator()(size_t idx, const char* fieldname) {
		if (idx >= numel()) {
			throw(extras::stacktrace_error("MxStruct::operator() index exceeds struct array dimension"));
		}
		return FieldWrapper(*this, idx, fieldname);
	}

}}
