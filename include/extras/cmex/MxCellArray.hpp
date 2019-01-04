#pragma once

#include "mxobject.hpp"
#include <vector>
#include <list>

namespace extras{namespace cmex{


    /// Wrapper around mxArray struct field elements
    /// provides simple get and set support using operator=
    class CellWrapper{
    protected:
        mxArray* parent;
        size_t index;
    public:
        CellWrapper()=default;
        CellWrapper(const CellWrapper& src) = default;
        CellWrapper(CellWrapper&& src) = default;

        CellWrapper(mxArray* par,size_t idx)
        {
            parent = par;
            index = idx;
        }

        /// get cell
        operator const mxArray* () const
        {
            return mxGetCell(parent, index);
        }

        /// set field
        CellWrapper& operator=(mxArray* pvalue){
            mxSetCell(parent, index, pvalue);
            return *this;
        }

        /// set field equal to string
        CellWrapper& operator=(const char* str){
            mxSetCell(parent,index,mxCreateString(str));
            return *this;
        }
    };



    /// Extension of MxObject for struct array support
    /// fields can be accessed using a simple syntax:
    ///     StructArray(index,"FieldName") = ...//pointer to mxArray to set
    ///     const mxArray* field = StructArray(index,"FieldName"); //get pointer to field
    class MxCellArray:public MxObject{

    public:
        //////////////
        // Constructors

        MxCellArray(){
            mxDestroyArray(_mxptr);
            _mxptr = mxCreateCellMatrix(0, 0);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
        }

        MxCellArray(size_t nStr, const char* str[]){
            mxDestroyArray(_mxptr);
            _mxptr = mxCreateCellMatrix(nStr, 1);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;

            for(size_t n=0;n<nStr;++n){
                mxSetCell(_mxptr,n,mxCreateString(str[n]));
            }
        }

        MxCellArray(const std::vector<std::string>& str){
            mxDestroyArray(_mxptr);
            _mxptr = mxCreateCellMatrix(str.size(), 1);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;

            size_t n=0;
            for( auto& s : str){
                mxSetCell(_mxptr,n,mxCreateString(s.c_str()));
                ++n;
            }
        }

        MxCellArray(const std::list<std::string>& str){
            mxDestroyArray(_mxptr);
            _mxptr = mxCreateCellMatrix(str.size(), 1);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;

            size_t n=0;
            for( auto& s : str){
                mxSetCell(_mxptr,n,mxCreateString(s.c_str()));
                ++n;
            }
        }

        MxCellArray(const MxObject& src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray(const MxObject& src): src is not a cell."));
            }
            copyFrom(src);
        }
        MxCellArray(MxObject&& src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray(MxObject&& src): src is not a cell."));
            }
            moveFrom(src);
        }

        /// copy assignment
        MxCellArray& operator=(const MxObject& src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray::operator=(const MxObject& src): src is not a cell."));
            }
			copyFrom(src);
            return *this;
        }

        /// move assignment
        MxCellArray& operator=(MxObject && src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray::operator=(MxObject && src): src is not a cell."));
            }
			moveFrom(src);
            return *this;
        }

        /// Construct and assign from mxarray
        /// assume mxArray* is not persistent
        MxCellArray(mxArray* mxptr,bool isPersistent=false):
            MxObject(mxptr,isPersistent)
        {
            //mexPrintf("mxObject(mx*):%d fromL %d\n",this,mxptr);
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a cell."));
            }
        }
        MxCellArray& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxCellArray(mxArray* mxptr,bool isPersistent=false): src is not a cell."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }
        MxCellArray(const mxArray* mxptr, bool isPersistent = false):
            MxObject(mxptr,isPersistent)
        {
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxCellArray& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }

        //////////////////
        // Field Access

        /// non-const access to field
        CellWrapper operator()(size_t idx){
            if(_setFromConst){
                throw(std::runtime_error("MxCellArray::operator() Cannot get non-const access element of cell set from constant."));
            }
            if(idx>=mxGetNumberOfElements(_mxptr)){
                throw(std::runtime_error("MxCellArray::operator() index exceeds struct array dimension"));
            }
            
            return CellWrapper(_mxptr,idx);
        }

        /// const access to field
        const mxArray* operator()(size_t idx) const{
            if(idx>=mxGetNumberOfElements(_mxptr)){
                throw(std::runtime_error("MxCellArray::operator() index exceeds struct array dimension"));
            }
            return mxGetCell(_mxptr,idx);
        }


    };


}}
