#pragma once

#include <mex.h>
#include <exception>
#include <vector>
#include <string>
#include <initializer_list>
#include "mexextras.hpp"

namespace extras{namespace cmex{

	///Exceptions thrown by MxObject
    class MxObjectException: public std::exception{
        virtual const char * what() const throw(){
            return "Generic Exception thrown by MxObject Class";
        }
    };

    class MxNotStruct: MxObjectException{
        const char * what() const throw(){
            return "mxStruct syntax used on non-struct MxObject";
        }
    };
    class MxNotNumeric: MxObjectException{
        const char * what() const throw(){
            return "mxNumeric syntax used on non-numeric MxObject";
        }
    };
    class MxNotCell: MxObjectException{
        const char * what() const throw(){
            return "mxCell syntax used on non-cell MxObject";
        }
    };

	//template<typename M> class NumericArray;

	///Object wrapper around mxArray*
    class MxObject{
		//template<typename M> friend class NumericArray;
    protected:
        mxArray* _mxptr = nullptr; //pointer to mxArray holding data
        mutable bool _managemxptr = true; //flag specifying if class should delete _mxptr upon destruction
		bool _isPersistent = false;
		bool _setFromConst = false;

        void deletemxptr(){
            if(_managemxptr && _mxptr!=nullptr){
                //mexPrintf("mxObject().deletemxpty():%d _mxptr=%d\n",this,_mxptr);
                mxDestroyArray(_mxptr);
                _mxptr = nullptr;
				_isPersistent = false;
            }
        }

		virtual void copyFrom(const MxObject& src) {
			//mexPrintf("MxObject::copyFrom: this=%d src=%d\n", this, &src);
			deletemxptr();
			if (src._mxptr == nullptr) {
				_mxptr = nullptr;
				_managemxptr = true;
				_isPersistent = false;
				_setFromConst = false;
			}
			else {
				_mxptr = mxDuplicateArray(src._mxptr);
				_managemxptr = true;
				_setFromConst = false;
				if (_isPersistent) {
					mexMakeArrayPersistent(_mxptr);
				}
				else if (src.isPersistent()) {
					mexMakeArrayPersistent(_mxptr);
					_isPersistent = true;
				}
			}
		}

        virtual void moveFrom(MxObject& src) {
			//mexPrintf("MxObject::moveFrom: this=%d src=%d\n", this, &src);

			deletemxptr();
			if (src._mxptr == nullptr) {
				_mxptr = nullptr;
				_managemxptr = true;
				_setFromConst = false;
			}
			else {
				_mxptr = src._mxptr;
				_managemxptr = src._managemxptr;
				_setFromConst = src._setFromConst;
				src._managemxptr = false;
				_isPersistent = src._isPersistent;
			}
		}
    public:
		//// Constructors for persistent Data

		// create persistent mxArray with empty data
		static MxObject createPersistent() {
			MxObject out;
			out.makePersistent();
			return out;
		}

		// create persistent mxArray by copying array
		static MxObject createPersistent(const MxObject& src) {
			MxObject out(src);
			out.makePersistent();
			return out;
		}
		static MxObject createPersistent(mxArray* src, bool isPersistent = false) {
			MxObject out(src, isPersistent);
			out.makePersistent();
			return out;
		}
		static MxObject createPersistent(const mxArray* src, bool isPersistent = false) {
			MxObject out(src, isPersistent);
			out.makePersistent();
			return out;
		}
		// create persistent scalar
		static MxObject createPersistent(const double& in) {
			MxObject out(in);
			out.makePersistent();
			return out;
		}
		// create persistent string
		static MxObject createPersistent(const std::string & in) {
			MxObject out(in);
			out.makePersistent();
			return out;
		}

		/// make mxArray persistent so that it survives beyond each call to the initializing mexFunction
		void makePersistent() {
			if (_isPersistent) { return; }
			if (!_managemxptr) {
				_mxptr = mxDuplicateArray(_mxptr);
				_managemxptr = true;
				_setFromConst = false;
			}
			mexMakeArrayPersistent(_mxptr);
			_isPersistent = true;
		}

		///true if mxArray is persistent (i.e. can outlive a mexFunction call)
		bool isPersistent() const { return _isPersistent; }

        //destructor
        ~MxObject(){
            deletemxptr();
            //mexPrintf("~mxObject():%d _mxptr:%d\n",this,_mxptr);
        }

        ///Generic constructor, constructs empty array
        MxObject(){
            //mexPrintf("mxObject():%d\n",this);
            _mxptr = mxCreateDoubleMatrix(0,0,mxREAL);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
        }

        /// Copy constructor and copy assignment
        MxObject(const MxObject& src){
			copyFrom(src);
        }

		/// copy assignment
        MxObject& operator=(const MxObject& src){
			copyFrom(src);
            return *this;
        }

        /// move constructor and move assignment
        MxObject(MxObject && src){
			moveFrom(src);
        }

		/// move assignment
        MxObject& operator=(MxObject && src){
			moveFrom(src);
            return *this;
        }

        /// Construct and assign from mxarray
        /// assume mxArray* is not persistent
        MxObject(mxArray* mxptr,bool isPersistent=false){
            //mexPrintf("mxObject(mx*):%d fromL %d\n",this,mxptr);
            _mxptr = mxptr;
            _managemxptr = false;
			_isPersistent = isPersistent;
			_setFromConst = false;
        }
        MxObject& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            deletemxptr();
            _mxptr = mxptr;
            _managemxptr = false;
			_isPersistent = false;
			_setFromConst = false;
            return *this;
        }
        MxObject(const mxArray* mxptr, bool isPersistent = false){
            //mexPrintf("mxObject(const mx*):%d from: %d\n",this,mxptr);
            /*if(mxptr!=nullptr)
                _mxptr = mxDuplicateArray(mxptr);
            _managemxptr = true;*/
            _mxptr = (mxArray*)mxptr; //force const conversion
            _managemxptr = false;
			_isPersistent = isPersistent;
			_setFromConst = true;
        }
        MxObject& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            deletemxptr();

            /*if(mxptr!=nullptr)
                _mxptr = mxDuplicateArray(mxptr);
            _managemxptr = true;*/
            _mxptr = (mxArray*)mxptr; //force const conversion
            _managemxptr = false;
			_isPersistent = false;
			_setFromConst = true;
            return *this;
        }


        // Special Constructors & assigment operators

        /// double scalar constructor
        MxObject(const double& in){
            _mxptr = mxCreateDoubleScalar(in);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
        }

		/// set to double scalar
        MxObject& operator=(const double& in){
            deletemxptr();
            _mxptr = mxCreateDoubleScalar(in);
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
            return *this;
        }

        //string
        MxObject(const std::string & in){
            _mxptr = mxCreateString(in.c_str());
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
        }
        MxObject& operator=(const std::string& in){
            deletemxptr();
            _mxptr = mxCreateString(in.c_str());
            _managemxptr = true;
			_isPersistent = false;
			_setFromConst = false;
            return *this;
        }

        /// assign to (cast to) mxArray
		///if array is not persistent change management rule so that data is not deleted on destruction
		///if array is persistent, returns copy of the data that is not persistent (and therefore managed by MATLAB)
        operator mxArray*(){
			if (_isPersistent||_setFromConst) {
				return mxDuplicateArray(_mxptr);
			}
            _managemxptr = false;
            return _mxptr;
        }

		/// returns const mxArray*, memory management is not changed
		/// pointer may be invalid when object is deleted
        operator const mxArray*() const{
            return _mxptr;
        }

        ///Change data ownership
        virtual void managedata(){
            if(!_managemxptr){
                _mxptr = mxDuplicateArray(_mxptr);
                _managemxptr = true;
				_isPersistent = false;
				_setFromConst = false;
            }
        }

        /// return data type held by mxArray pointer
        mxClassID mxType() const{
            if(_mxptr==nullptr){
                return mxUNKNOWN_CLASS;
            }
            return mxGetClassID(_mxptr);
        }

        ///return the mxArray ptr without changing data managment rule
        /// if data is locally held, it will be deleted upon construction
        /// DO NOT use for setting the return data of a mexFunction.
        ///instead use the cast operator:
        /// e.g
        ///  prhs[0] = (mxArray *)YourMxObject;
        const mxArray* getmxarray() const{
            return _mxptr;
        }

        ///return the mxArray and change data management rule to false
        /// after calling, the mxObject will no longer delete the mxArray
        /// when it goes out of scope. Therefore it is the user's responsibility
        /// to manage the memory appropriately.
        /// WARNING: this is meant for advanced usage, be sure you understand before using.
        /// Note: if the mxArray was persistent it will remain persistent.
        /// the user can check if the mxArray was persistent using mxObj.isPersistent()
        /// Also, it is possible the mxArray was not ever under this mxObject's management
        /// in that case the user probably does not want to delete the data
        mxArray* releasemxarray(bool* wasManaged = nullptr, bool* wasPersistent=nullptr,bool* wasSetFromConst=nullptr){
            if(wasPersistent!=nullptr){
                *wasPersistent = isPersistent();
            }
            if(wasManaged != nullptr){
                *wasManaged = _managemxptr;
            }
			if (wasSetFromConst != nullptr) {
				*wasSetFromConst = _setFromConst;
			}
            _managemxptr = false;
            return _mxptr;
        }

        /// std::vector holding dimension of the mxArray object
        virtual std::vector<size_t> size() const{
            return cmex::size(_mxptr);
        }

        /// number of elements
        virtual size_t numel() const{
            return cmex::numel(_mxptr);
        }

        /// true if mxArray is a struct
        bool isstruct() const{
            return mxIsStruct(_mxptr);
        }

		virtual mxArray* getField(size_t element, const char* fieldname) const {
			if (!isstruct()) { throw(std::runtime_error("MxObject::getField(): mxArray* is not a struct.")); }
			return mxGetField(_mxptr, element, fieldname);
		}


        /// true if is numeric
        bool isnumeric() const{
            return mxIsNumeric(_mxptr);
        }
        bool iscell() const{
            return mxIsCell(_mxptr);
        }
        bool ischar() const{
            return mxIsChar(_mxptr);
        }

        /// Resize
        void reshape(std::vector<size_t> dims){
            if(_setFromConst){
                throw(std::runtime_error("cannot reshape mxobject set from const mxArray*"));
            }

            mxArray* newPtr;
            switch(mxGetClassID(_mxptr)){
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
                    if(!mxIsComplex(_mxptr)){ //real data just a simple copy
                        newPtr = mxCreateNumericArray(dims.size(),dims.data(),mxGetClassID(_mxptr),mxREAL);
                        memcpy(mxGetData(newPtr),mxGetData(_mxptr),mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr),mxGetNumberOfElements(newPtr)));
                    }else{ // complex data
                        newPtr = mxCreateNumericArray(dims.size(),dims.data(),mxGetClassID(_mxptr),mxCOMPLEX);
                        #if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just use standard copy because elementsize is 2x
                        memcpy(mxGetData(newPtr),mxGetData(_mxptr),mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr),mxGetNumberOfElements(newPtr)));
                        #else
                        memcpy(mxGetData(newPtr),mxGetData(_mxptr),mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr),mxGetNumberOfElements(newPtr)));
                        memcpy(mxGetImagData(newPtr),mxGetImagData(_mxptr),mxGetElementSize(_mxptr)*std::min(mxGetNumberOfElements(_mxptr),mxGetNumberOfElements(newPtr)));
                        #endif
                    }
                    if(_managemxptr&&!_setFromConst){
                        mxDestroyArray(_mxptr);
                    }
                    _mxptr = newPtr;
                    _managemxptr = true;
                    _setFromConst = false;
                    if(_isPersistent){
                        mexMakeArrayPersistent(newPtr);
                    }
                    break;
                case mxCELL_CLASS:
                    newPtr = mxCreateCellArray(dims.size(), dims.data());
                    if(_managemxptr&&!_setFromConst){ //just move the arrays
                        for(size_t n=0;n<std::min(mxGetNumberOfElements(_mxptr),mxGetNumberOfElements(newPtr));++n){
                            mxSetCell(newPtr, n, mxGetCell(_mxptr,n));
                            mxSetCell(_mxptr,n,nullptr);
                        }
                        mxDestroyArray(_mxptr);
                    }else{ //need copy
                        for(size_t n=0;n<std::min(mxGetNumberOfElements(_mxptr),mxGetNumberOfElements(newPtr));++n){
                            mxSetCell(newPtr, n, mxDuplicateArray(mxGetCell(_mxptr,n)));
                        }
                    }
                    _mxptr = newPtr;
                    _managemxptr = true;
                    _setFromConst = false;
                    if(_isPersistent){
                        mexMakeArrayPersistent(newPtr);
                    }
                    break;
                case mxSTRUCT_CLASS:
                    mxSetDimensions(_mxptr,dims.data(),dims.size());
                    break;
            	default:
            		throw(std::runtime_error("reshape not implemented for class"));
        	}
        }

        /// Resize
        void reshape(size_t nRows, size_t nCols){
            reshape({nRows,nCols});
        }


		///returns true if ndims < 3
		virtual bool ismatrix() const { return  mxGetNumberOfDimensions(_mxptr)<3; }
		virtual size_t ndims() const { return mxGetNumberOfDimensions(_mxptr); }

        virtual bool isempty() const{ return numel()==0;}

    };

    // MxObject Info functions

    /// return data type held by mxArray pointer
    mxClassID mxType(const MxObject& mxo){return mxo.mxType();}

    std::vector<size_t> size(const MxObject& mxo){return mxo.size();}

    size_t numel(const MxObject& mxo){return mxo.numel();}

    bool isstruct(const MxObject& mxo){return mxo.isstruct();}
    bool isnumeric(const MxObject& mxo){ return mxo.isnumeric();}
    bool iscell(const MxObject& mxo){ return mxo.iscell();}
    bool ischar(const MxObject& mxo){return mxo.ischar();}

    std::string getstring(const MxObject& mxo){ return getstring(mxo.getmxarray());}

}}
