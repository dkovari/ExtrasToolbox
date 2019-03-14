/* MxCellArray.hpp
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
---------------------------------------------------*/

#pragma once

#include "mxobject.hpp"
#include <vector>
#include <list>

namespace extras{namespace cmex{

	class MxCellArray;
	class CellWrapper;

    //! Extension of MxObject for struct array support
    //! fields can be accessed using a simple syntax:
    //!     StructArray(index,"FieldName") = ...//pointer to mxArray to set
    //!     const mxArray* field = StructArray(index,"FieldName"); //get pointer to field
    class MxCellArray:public MxObject{
		friend class CellWrapper;
    public:
        //////////////
        // Constructors

		//! create empty cell array
        MxCellArray(){
            mxDestroyArray(_mxptr);
            _mxptr = mxCreateCellMatrix(0, 0);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
        }

		//! create cellstr array from array of const char*
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

		//! create cellstr array from vector of strings
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

		//! create cellstr from list of strings
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

		//! create cell array by copying src MxObject
        MxCellArray(const MxObject& src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray(const MxObject& src): src is not a cell."));
            }
            copyFrom(src);
        }

		//! move constructor
        MxCellArray(MxObject&& src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray(MxObject&& src): src is not a cell."));
            }
            moveFrom(src);
        }

        //! copy assignment
        MxCellArray& operator=(const MxObject& src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray::operator=(const MxObject& src): src is not a cell."));
            }
			copyFrom(src);
            return *this;
        }

        //! move assignment
        MxCellArray& operator=(MxObject && src){
            if(!src.iscell()){
                throw(std::runtime_error("MxCellArray::operator=(MxObject && src): src is not a cell."));
            }
			moveFrom(src);
            return *this;
        }

        //! Construct and assign from mxarray
        //! assume mxArray* is not persistent
        MxCellArray(mxArray* mxptr,bool isPersistent=false):
            MxObject(mxptr,isPersistent)
        {
            //mexPrintf("mxObject(mx*):%d fromL %d\n",this,mxptr);
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a cell."));
            }
        }

		//! assign from mxArray*
        MxCellArray& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxCellArray(mxArray* mxptr,bool isPersistent=false): src is not a cell."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }

		//! construct from const mxArray*
		//! optionally specify if array was persistent (default=false)
        MxCellArray(const mxArray* mxptr, bool isPersistent = false):
            MxObject(mxptr,isPersistent)
        {
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }

		//! assign from const mxArray*
        MxCellArray& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            if(!mxIsCell(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }

        //////////////////
        // Cell Access

        //! non-const access to field
		CellWrapper operator()(size_t idx);

		//! non-const access to field
		CellWrapper operator()(const std::vector<size_t>& subscripts);

        //! const access to field
        const mxArray* operator()(size_t idx) const{
            if(idx>=mxGetNumberOfElements(_mxptr)){
                throw(std::runtime_error("MxCellArray::operator() index exceeds struct array dimension"));
            }
            return mxGetCell(_mxptr,idx);
        }

		//! const access to field
		const mxArray* operator()(const std::vector<size_t>& subscripts) const {

			size_t idx = mxCalcSingleSubscript(_mxptr, subscripts.size(), subscripts.data());

			if (idx >= mxGetNumberOfElements(_mxptr)) {
				throw(std::runtime_error("MxCellArray::operator() index exceeds struct array dimension"));
			}
			return mxGetCell(_mxptr, idx);
		}
    };

	//! Wrapper around mxArray struct field elements
	//! provides simple get and set support using operator=
	class CellWrapper {
		friend class MxCellArray;
	protected:
		MxCellArray& _parent;
		size_t index;

		//! Construct CellWrapper (not copy/move)
		//! this should only be called by MxCellArray
		CellWrapper(MxCellArray& parent, size_t idx) :_parent(parent), index(idx) {};

	public:
		CellWrapper(const CellWrapper& src) = default;
		CellWrapper(CellWrapper&& src) = default;
		CellWrapper& operator=(const CellWrapper& src) = default;
		CellWrapper& operator=(CellWrapper&& src) = default;

		//! get cell
		operator const mxArray* () const
		{
			std::lock_guard<std::mutex> lock(_parent._mxptrMutex); //!< lock _mxptr;
			return mxGetCell(_parent._mxptr, index);
		}

		//! set field
		CellWrapper& operator=(mxArray* pvalue) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("MxCellArray::operator() Cannot set element of cell set from constant."));
			}
			mxDestroyArray(mxGetCell(_parent._mxptr, index));
			mxSetCell(_parent._mxptr, index, pvalue);
			return *this;
		}

		//! set field from const
		//! duplicates array
		CellWrapper& operator=(const mxArray* pvalue) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("MxCellArray::operator() Cannot set element of cell set from constant."));
			}
			mxDestroyArray(mxGetCell(_parent._mxptr, index));
			mxSetCell(_parent._mxptr, index, mxDuplicateArray(pvalue));
			return *this;
		}

		//! set field equal to string
		CellWrapper& operator=(const char* str) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("MxCellArray::operator() Cannot set element of cell set from constant."));
			}
			mxDestroyArray(mxGetCell(_parent._mxptr, index));
			mxSetCell(_parent._mxptr, index, mxCreateString(str));
			return *this;
		}

		//! set field equal to string
		CellWrapper& operator=(double val) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("MxCellArray::operator() Cannot set element of cell set from constant."));
			}
			mxDestroyArray(mxGetCell(_parent._mxptr, index));
			mxSetCell(_parent._mxptr, index, mxCreateDoubleScalar(val));
			return *this;
		}

		//! Move from MxObject
		CellWrapper& operator=(MxObject&& src) {
			if (_parent._setFromConst) {
				throw(std::runtime_error("CellWrapper: Cannot use operator= for CellWrapper created from const mxArray*"));
			}

			mxDestroyArray(mxGetCell(_parent._mxptr, index));
			mxSetCell(_parent._mxptr, index, src);
			return *this;

		}
	};

	////////////////////////////
	// Method definitions needing CellWrapper

	//! non-const access to field
	CellWrapper MxCellArray::operator()(size_t idx) {


		if (idx >= mxGetNumberOfElements(_mxptr)) {
			throw(std::runtime_error("MxCellArray::operator() index exceeds struct array dimension"));
		}

		return CellWrapper(*this, idx);
	}

	//! non-const access to field
	CellWrapper MxCellArray::operator()(const std::vector<size_t>& subscripts) {

		size_t idx = mxCalcSingleSubscript(_mxptr, subscripts.size(), subscripts.data());
		if (idx >= mxGetNumberOfElements(_mxptr)) {
			throw(std::runtime_error("MxCellArray::operator() index exceeds struct array dimension"));
		}

		return CellWrapper(*this, idx);
	}
}}
