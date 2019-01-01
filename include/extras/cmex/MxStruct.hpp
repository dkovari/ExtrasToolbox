#pragma once

#include "mxobject.hpp"

namespace extras{namespace cmex{
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
            if(mxIsStruct(_mxptr)){
                throw(std::runtime_error("MxStruct(const MxObject& src): src is not a struct."));
            }
        }
        MxStruct(MxObject&& src): MxObject(src){
            if(mxIsStruct(_mxptr)){
                throw(std::runtime_error("MxStruct(const MxObject& src): src is not a struct."));
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
            if(mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxStruct& operator=(mxArray* mxptr){
			//mexPrintf("MxObject& operator=(mx*):%d from: %d\n", this, mxptr);
            if(mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }
        MxStruct(const mxArray* mxptr, bool isPersistent = false):
            MxObject(mxptr,isPersistent)
        {
            if(mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
        }
        MxStruct& operator=(const mxArray* mxptr){
			//mexPrintf("MxObject& operator=(const mx*):%d from: %d\n", this, mxptr);
            if(mxIsStruct(mxptr)){
                throw(std::runtime_error("MxStruct(mxArray* mxptr,bool isPersistent=false): src is not a struct."));
            }
            MxObject::operator=(mxptr);
            return *this;
        }

        //////////////////
        // Field Access


    };

    class FieldWrapper{
    protected:
        mxArray* parentStruct;
    public:
        CellFieldWrapper(mxArray* mxptr)
    }
}}
