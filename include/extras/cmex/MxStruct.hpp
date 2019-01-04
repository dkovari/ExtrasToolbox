#pragma once

#include "mxobject.hpp"

namespace extras{namespace cmex{


    /// Wrapper around mxArray struct field elements
    /// provides simple get and set support using operator=
    class FieldWrapper{
    protected:
        mxArray* parent=nullptr;
        int field_number=-1;
        size_t index=0;
    public:
		FieldWrapper() = default;
        FieldWrapper(const FieldWrapper& src) = default;
        FieldWrapper(FieldWrapper&& src) = default;
        FieldWrapper& operator=(const FieldWrapper& src) = default;
        FieldWrapper& operator=(FieldWrapper&& src) = default;

        FieldWrapper(mxArray* parentPtr,size_t idx, const char* fieldname):
            parent(parentPtr),
            index(idx)
        {
            field_number = mxGetFieldNumber(parent,fieldname);
            if(field_number<0){ //need to create field
                field_number = mxAddField(parent,fieldname);
            }

            if(field_number<0){
                throw(std::runtime_error(std::string("FieldWrapper(): Could not create fieldname:")+std::string(fieldname)));
            }
        }

        FieldWrapper(mxArray* parentPtr,size_t idx, int fieldnumber):
            parent(parentPtr),
            field_number(fieldnumber),
            index(idx)
        {
            if(field_number<0){
                throw(std::runtime_error("invalid field number"));
            }
        }

        /// get field
        operator const mxArray*() const{
            return mxGetFieldByNumber(parent,index,field_number);
        }

        /// set field
        FieldWrapper& operator=(mxArray* pvalue){
            mxDestroyArray(mxGetFieldByNumber(parent,index,field_number));
            mxSetFieldByNumber(parent,index,field_number,pvalue);
            return *this;
        }

		/// set field from const array
		/// duplicated array
		FieldWrapper& operator=(const mxArray* pvalue) {
			mxDestroyArray(mxGetFieldByNumber(parent, index, field_number));
			mxSetFieldByNumber(parent, index, field_number, mxDuplicateArray(pvalue));
			return *this;
		}

        /// set field equal to string
        FieldWrapper& operator=(const char* str){
            mxDestroyArray(mxGetFieldByNumber(parent,index,field_number));
            mxSetFieldByNumber(parent,index,field_number,mxCreateString(str));
            return *this;
        }

        /// set field equal to scalar double
        FieldWrapper& operator=(double val){
            mxDestroyArray(mxGetFieldByNumber(parent,index,field_number));
            mxSetFieldByNumber(parent,index,field_number,mxCreateDoubleScalar(val));
            return *this;
        }
    };



    /// Extension of MxObject for struct array support
    /// fields can be accessed using a simple syntax:
    ///     StructArray(index,"FieldName") = ...//pointer to mxArray to set
    ///     const mxArray* field = StructArray(index,"FieldName"); //get pointer to field
    class MxStruct:public MxObject{

    public:
        //////////////
        // Constructors

        MxStruct(){
            mxDestroyArray(_mxptr);
            _mxptr = mxCreateStructMatrix(0, 0, 0, nullptr);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
        };

        MxStruct(const MxObject& src): MxObject(src){
            if(!mxIsStruct(_mxptr)){
                throw(std::runtime_error("MxStruct(const MxObject& src): src is not a struct."));
            }
        }
        MxStruct(MxObject&& src): MxObject(std::move(src)){
            if(!mxIsStruct(_mxptr)){
                throw(std::runtime_error("MxStruct(MxObject&& src): src is not a struct."));
            }
        }

        /// copy assignment
        MxStruct& operator=(const MxObject& src){
            if(!src.isstruct()){
                throw(std::runtime_error("MxStruct::operator=(const MxObject& src): src is not a struct."));
            }
			copyFrom(src);
            return *this;
        }

        /// move assignment
        MxStruct& operator=(MxObject && src){
            if(!src.isstruct()){
                throw(std::runtime_error("MxStruct::operator=(MxObject && src): src is not a struct."));
            }
			moveFrom(src);
            return *this;
        }

        /// Construct and assign from mxarray
        /// assume mxArray* is not persistent
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

        //////////////////
        // Field Access

        /// non-const access to field
        FieldWrapper operator()(size_t idx, const char* fieldname){
            if(_setFromConst){
                throw(std::runtime_error("MxCellArray::operator() Cannot get non-const access element of cell set from constant."));
            }
            if(idx>=mxGetNumberOfElements(_mxptr)){
                throw(std::runtime_error("MxStruct::operator() index exceeds struct array dimension"));
            }
            return FieldWrapper(_mxptr,idx,fieldname);
        }

        /// const access to field
        const mxArray* operator()(size_t idx, const char* fieldname) const{
            if(idx>=mxGetNumberOfElements(_mxptr)){
                throw(std::runtime_error("MxStruct::operator() index exceeds struct array dimension"));
            }
            return mxGetField(_mxptr,idx,fieldname);
        }

    };


}}
