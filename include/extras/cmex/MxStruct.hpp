#pragma once

#include "mxobject.hpp"

namespace extras{namespace cmex{


    /// Wrapper around mxArray struct field elements
    /// provides simple get and set support using operator=
    class FieldWrapper{
    protected:
        mxArray* parentStruct=nullptr;
        int field_number=-1;
        size_t index=0;
    public:
        FieldWrapper();
        FieldWrapper(const FieldWrapper& src) = default;
        FieldWrapper(FieldWrapper&& src) = default;
        FieldWrapper& operator=(const FieldWrapper& src) = default;
        FieldWrapper& operator=(FieldWrapper&& src) = default;

        FieldWrapper(mxArray* parentStructPtr,size_t idx, const char* fieldname):
            parentStruct(parentStructPtr),
            index(idx)
        {
            field_number = mxGetFieldNumber(parentStruct,fieldname);
            if(field_number<0){ //need to create field
                field_number = mxAddField(parentStruct,fieldname);
            }

            if(field_number<0){
                throw(std::runtime_error(std::string("FieldWrapper(): Could not create fieldname:")+std::string(fieldname)));
            }
        }

        FieldWrapper(mxArray* parentStructPtr,size_t idx, int fieldnumber):
            parentStruct(parentStructPtr),
            field_number(fieldnumber),
            index(idx)
        {
            if(field_number<0){
                throw(std::runtime_error("invalid field number"));
            }
        }

        /// get field
        operator const mxArray*() const{
            return mxGetFieldByNumber(parentStruct,index,field_number);
        }

        /// set field
        FieldWrapper& operator=(mxArray* pvalue){
            mxSetFieldByNumber(parentStruct,index,field_number,pvalue);
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

        /// Resize
        void reshape(std::vector<size_t> dims){
            mxSetDimensions(_mxptr,dims.data(),dims.size());
        }

        /// Resize
        void reshape(size_t nRows, size_t nCols){
            size_t dims[] = {nRows,nCols};
            mxSetDimensions(_mxptr,dims,2);
        }

    };


}}
