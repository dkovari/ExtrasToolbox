/*-----------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
------------------------------------------------------*/
/** mexFunction Class wrapper *************************************************
* Use this to wrap c++ objects so that their methods can be called from MATLAB
******************************************************************************/
#pragma once

#include <extras/cmex/mexextras.hpp>
#include "ObjectManager.h"
#include <extras/string_extras.hpp>
#include <functional>
#include <unordered_map>

namespace extras{namespace SessionManager{
	typedef std::unordered_map<std::string, std::function<void(int, mxArray*[], int, const mxArray*[])>> MapT_mexI; /// map type used in mexInterface

    /** mexFunction Class wrapper
    * Use this to wrap c++ objects so that their methods can be called from MATLAB
    *
    * Usage:
    * Your mex cpp file should look like this
    *
	*	  extras::SessionManager::ObjectManager<YOUR_CLASS_TYPE> manager; //object manager, persists between mexFunction calls
    *     mexInterface<YOUR_CLASS_TYPE,manager> MexInt; //global instance of mexInterface, persists between mexFunction calls
    *
    *     void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    *         MexInt.mexFunction(nlhs,plhs,nrhs,prhs);
    *     }
    *
    * In MATLAB you would call
    *     >> p_obj = YOUR_MEX_FUNCTION('new'); %get pointer to c++ object
    *     >> YOUR_MEX_FUNCTION('your_method',p_obj,Arg1,Arg2,...); %run method, note: must have extended mexInterface to implement your_method
    *     >> YOUR_MEX_FUNCTION('delete',p_obj); %delete associated object
	*
	* You can get A list of the methods by calling
	*	  >> metNames = YOU_MEX_FUNCTION('getMethodNames');
	* which will return a cellstr array specifying all of the method names
    *
    * Extending mexInterface:
    *  You should extend mexInterface for your object as follows
    *
	*     // <template extras::SessionManager::ObjectManager<ObjType>& manager> // <-------- optionlly make the manager field a template so that class can be defined in a header
    *     class yourMexInterface: public mexInterface<yourClass,manager>{ //note: if you are not using templated version manager corresponds to global object manager
    *         /// implement function named 'your_method'
    *         void your_method(int nlhs, mxArray* plhs[],int nrhs, const mxArray* prhs[]){
    *            if (nrhs < 1) {
    *                 throw(std::runtime_error("requires intptr argument specifying object to destruct"));
    *             }
    *             ObjManager.get(prhs[0])->yourMethod(...); //yourClass should have public member function yourMethod(...)
    *         }
    *     public:
    *         yourMexInterface(){
    *             using namespace std::placeholders;
    *             addFunction('your_method',std::bind(&yourMexInterface::your_method,*this,_1,_2,_3,_4)); //add your_method to function list
    *         }
    *     }
    */
    template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager>
    class mexInterface{
    private:
		MapT_mexI functionMap; //maps method name to class function
    protected:
        /// implement 'new' function interface
        void new_object(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
#ifdef DAN_DEBUG
			mexPrintf("mexInterface<%s>::new_object\n", typeid(ObjType).name());
			mexPrintf("\t press a key to continue\n");
			mexEvalString("pause()");
#endif
            int64_t val = ObjManager.create(new ObjType); //CHANGE THIS LINE
#ifdef DAN_DEBUG
			mexPrintf("\tObjPtr: %p\n", val);
			mexPrintf("\t press a key to continue\n");
			mexEvalString("pause()");
#endif
            plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
            *((int64_t*)mxGetData(plhs[0])) = val;
        }

        /// implement 'delete' function interface
        void delete_object(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[])
        {
            if (nrhs < 1) {
                throw(std::runtime_error("requires intptr argument specifying object to destruct"));
            }
            ObjManager.destroy(prhs[0]);
        }

        /// method for adding name-function pair to the methods list
        void addFunction(std::string name, std::function<void(int,mxArray**,int,const mxArray* *)> func){
            functionMap.insert(MapT_mexI::value_type(name, func));
        }

        /// Exception handler for errors thrown when executing a method from matlab
        void handle_exception(std::exception_ptr eptr,std::string functionName){
            using namespace std;
            try {
    			if (eptr) {
    				rethrow_exception(eptr);
    			}
    		}
    		catch (const exception& e) {
    			std::string source;
                source+= string("mexInterface:")+functionName;
    			mexErrMsgIdAndTxt(source.c_str(),"mexInterface<%s>::%s\nCaught exception: %s", typeid(ObjType).name(),functionName.c_str(),e.what());
    		}
    	}

        /// helper function for getting object instance from mxArray* holding int64_ptr
        /// Returns a shared pointer to the object instance
        std::shared_ptr<ObjType> getObjectPtr(int nrhs, const mxArray *prhs[]){
            if (nrhs < 1) {
                throw(std::runtime_error("requires intptr argument specifying object to destruct"));
            }
            return ObjManager.get(prhs[0]);
        }

		void clearObjects(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
#ifdef _DEBUG
			mexPrintf("Clearing All Objects.\n");
			mexEvalString("pause(0.2)");
#endif
			ObjManager.clearObjects();
		}

		void getMethodNames(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			mxArray* out = mxCreateCellMatrix(functionMap.size(), 1);
			size_t k = 0;
			for (auto& f : functionMap) {
				mxSetCell(out, k, mxCreateString(f.first.c_str()));
				k++;
			}
			plhs[0] = out;
		}
    public:

		virtual ~mexInterface() {
#ifdef _DEBUG
			mexPrintf("Destroying mexInterface<%s,...>\n", typeid(ObjType).name());
			mexEvalString("pause(0.2)");
#endif
		}

        mexInterface(){
            using namespace std::placeholders;

            addFunction("new",std::bind(&mexInterface::new_object,this,_1,_2,_3,_4));  //add 'new' command
            addFunction("delete",std::bind(&mexInterface::delete_object,this,_1,_2,_3,_4));  //add 'delete' command
			addFunction("clear_mex_objects", std::bind(&mexInterface::clearObjects, this, _1, _2, _3, _4));  //add 'clear_mex_objects' command
			addFunction("getMethodNames", std::bind(&mexInterface::getMethodNames, this, _1, _2, _3, _4)); //return cellstr specifying method names

        }

        void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
            /// validate first arg
            if (nrhs < 1 || !mxIsChar(prhs[0])){
                mexErrMsgIdAndTxt("mexInterface:argumentError", "Invalid argument: missing method name.");
            }
            std::string funcName;
            try{
                funcName = extras::cmex::getstring(prhs[0]);
#ifdef DAN_DEBUG
				mexPrintf("mexInterface<%s>::mexFunction\n", typeid(ObjType).name());
				mexPrintf("\tfuncName:%s\n", funcName.c_str());
				mexPrintf("\t press a key to continue\n");
				mexEvalString("pause()");
#endif
                auto search = functionMap.find(funcName);
                if (search != functionMap.end()) {
#ifdef DAN_DEBUG
					mexPrintf("\t  Entering Function\n");
					mexPrintf("\t press a key to continue\n");
					mexEvalString("pause()");
#endif
                    (search->second)(nlhs,plhs,nrhs-1,&(prhs[1])); //execute command
                }
                else {
                    throw(std::runtime_error(
                        std::string("'")+funcName+std::string("' method not found.")
                    ));
                }
            }catch(...){
                handle_exception(std::current_exception(),funcName);
            }
        }
    };
}}
