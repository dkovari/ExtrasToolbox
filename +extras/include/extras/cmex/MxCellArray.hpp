/* MxCellArray.hpp
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
---------------------------------------------------*/

#pragma once

#include "mxobject.hpp"
#include "MxWrapper.hpp"
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
			own(mxCreateCellMatrix(1, 0));
        }

		//! create cellstr array from array of const char*
        MxCellArray(size_t nStr, const char* str[]){
            own(mxCreateCellMatrix(nStr, 1));

            for(size_t n=0;n<nStr;++n){
                mxSetCell(getmxarray(),n,mxCreateString(str[n]));
            }
        }

		//! create cellstr array from vector of strings
        MxCellArray(const std::vector<std::string>& str){
            own(mxCreateCellMatrix(str.size(), 1));
            
            size_t n=0;
            for( auto& s : str){
                mxSetCell(getmxarray(),n,mxCreateString(s.c_str()));
                ++n;
            }
        }

		//! create cellstr from list of strings
        MxCellArray(const std::list<std::string>& str){
            own(mxCreateCellMatrix(str.size(), 1));
          
            size_t n=0;
            for( auto& s : str){
                mxSetCell(getmxarray(),n,mxCreateString(s.c_str()));
                ++n;
            }
        }

		//! create cell array by copying src MxObject
        MxCellArray(const MxObject& src){
            if(!src.iscell()){
                throw(extras::stacktrace_error("MxCellArray(const MxObject& src): src is not a cell."));
            }
            copyFrom(src);
        }

		//! move constructor
        MxCellArray(MxObject&& src){
            if(!src.iscell()){
                throw(extras::stacktrace_error("MxCellArray(MxObject&& src): src is not a cell."));
            }
            moveFrom(src);
        }

		//! construct with specified dimensions
		MxCellArray(const std::vector<size_t>& dims) {
			own(mxCreateCellArray(dims.size(), dims.data()));
		}

        //! copy assignment
        MxCellArray& operator=(const MxObject& src){
            if(!src.iscell()){
                throw(extras::stacktrace_error("MxCellArray::operator=(const MxObject& src): src is not a cell."));
            }
			copyFrom(src);
            return *this;
        }

        //! move assignment
        MxCellArray& operator=(MxObject && src){
            if(!src.iscell()){
                throw(extras::stacktrace_error("MxCellArray::operator=(MxObject && src): src is not a cell."));
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
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a cell."));
            }
        }

		//! assign from mxArray*
        MxCellArray& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            if(!mxIsCell(mxptr)){
                throw(extras::stacktrace_error("MxCellArray(mxArray* mxptr,bool isPersistent=false): src is not a cell."));
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
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }

		//! assign from const mxArray*
        MxCellArray& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            if(!mxIsCell(mxptr)){
                throw(extras::stacktrace_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
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
            if(idx>=numel()){
                throw(extras::stacktrace_error("MxCellArray::operator() index exceeds struct array dimension"));
            }
            return mxGetCell(getmxarray(),idx);
        }

		//! const access to field
		const mxArray* operator()(const std::vector<size_t>& subscripts) const {
			return mxGetCell(getmxarray(), sub2ind(subscripts));
		}
    };

	//! Wrapper around mxArray struct field elements
	//! provides simple get and set support using operator=
	class CellWrapper: public MxWrapper {
		friend class MxCellArray;
	protected:
		MxCellArray& _parent;
		size_t index;

		//! Construct CellWrapper (not copy/move)
		//! this should only be called by MxCellArray
		CellWrapper(MxCellArray& parent, size_t idx) :_parent(parent), index(idx) {};

		//! set the linked cell
		//! NOTE: src will be managed by the cell array after this, so don't try to delete or rely on src after calling internalSet()
		void internalSet(mxArray* src) {
			if (_parent.isConst()) {
				throw(extras::stacktrace_error("CellWrapper: Cannot use operator= for MxCellArray created from const mxArray*"));
			}
			mxDestroyArray(mxGetCell(_parent.getmxarray(), index));
			mxSetCell(_parent.getmxarray(), index, src);
		}

		virtual mxArray* internalGet() {
			if (_parent.isConst()) {
				throw(extras::stacktrace_error("CellWrapper: Cannot return mxArray* for specified field  because parent cell is const."));
			}
			return mxGetCell(_parent.getmxarray(), index);
		}
		virtual const mxArray* internalGet() const {
			return mxGetCell(_parent.getmxarray(), index);
		}

	public:
		using MxWrapper::operator=; //%pull other assignment operators from MxWrapper

		//CellWrapper(const CellWrapper& src) = default;
		CellWrapper(CellWrapper&& src) = default;
		//CellWrapper& operator=(const CellWrapper& src) = default;
		CellWrapper& operator=(CellWrapper&& src) = default;

		
	};

	////////////////////////////
	// Method definitions needing CellWrapper

	//! non-const access to field
	CellWrapper MxCellArray::operator()(size_t idx) {
		if (idx >=numel()) {
			throw(extras::stacktrace_error("MxCellArray::operator() index exceeds struct array dimension"));
		}
		return CellWrapper(*this, idx);
	}

	//! non-const access to field
	CellWrapper MxCellArray::operator()(const std::vector<size_t>& subscripts) {
		return CellWrapper(*this, sub2ind(subscripts));
	}
}}
