#pragma once

#include <extras/cmex/mexextras.hpp>
#include "ObjectManager.h"
#include <extras/string_extras.hpp>
#include <functional>
#include <unordered_map>

namespace extras {namespace SessionManager {

    

	/** 
    * @brief mexFuncton Class manager wrapper.
    * 
    * Use this to wrap c++ objects so that their methods can be called from MATLAB
    *
    * Usage:
    * Your mex cpp file should look like this
    *
    *     `mexInterfaceManager<YOUR_CLASS_TYPE> MexInt; //global instance of mexInterface, persists between mexFunction calls
    *
    *     void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    *         MexInt.mexFunction(nlhs,plhs,nrhs,prhs);
    *     }`
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
    *    `class yourMexInterface: public mexInterfaceManager<yourClass>{
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
    *     };`
	 * @tparam ObjType the class type you want to manager
	*/
	template<class ObjType>
	class mexInterfaceManager: virtual protected ObjectManager<ObjType>{
        typedef std::unordered_map<std::string, std::function<void(int, mxArray* [], int, const mxArray* [])>> MapT_mexI; /// map type used in mexInterface
    /*public:
        struct memberGetSet {
            std::function<void(int, mxArray* [], int, const mxArray* [])> set;
            std::function<void(int, mxArray* [], int, const mxArray* [])> get;
        };*/
    private:
        MapT_mexI functionMap; //maps method name to class function
    protected:
        /// implement 'new' function interface
        virtual void new_object(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
#ifdef DAN_DEBUG
            mexPrintf("mexInterfaceManager<%s>::new_object\n", typeid(ObjType).name());
            mexPrintf("\t press a key to continue\n");
            mexEvalString("pause()");
#endif
            int64_t val = ObjectManager<ObjType>::create(new ObjType); //CHANGE THIS LINE
#ifdef DAN_DEBUG
            mexPrintf("\tObjPtr: %p\n", val);
            mexPrintf("\t press a key to continue\n");
            mexEvalString("pause()");
#endif
            plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
            *((int64_t*)mxGetData(plhs[0])) = val;
        }

        /// implement 'delete' function interface
        void delete_object(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
        {
            if (nrhs < 1) {
                throw(extras::stacktrace_error("requires intptr argument specifying object to destruct"));
            }
            ObjectManager<ObjType>::destroy(prhs[0]);
        }

        /// method for adding name-function pair to the methods list
        virtual void addFunction(std::string name, std::function<void(int, mxArray**, int, const mxArray**)> func) {
            functionMap.insert(MapT_mexI::value_type(name, func));
        }

        /// Exception handler for errors thrown when executing a method from matlab
        void handle_exception(std::exception_ptr eptr, std::string functionName) {
            using namespace std;
            try {
                if (eptr) {
                    rethrow_exception(eptr);
                }
            }
            catch (const exception& e) {
                std::string source;
                source += string("mexInterfaceManager:") + functionName;
                mexErrMsgIdAndTxt(source.c_str(), "mexInterfaceManager<%s>::%s\nCaught exception: %s", typeid(ObjType).name(), functionName.c_str(), e.what());
            }
        }

        /// helper function for getting object instance from mxArray* holding int64_ptr
        /// Returns a shared pointer to the object instance
        std::shared_ptr<ObjType> getObjectPtr(int nrhs, const mxArray* prhs[]) {
            if (nrhs < 1) {
                throw(extras::stacktrace_error("requires intptr argument specifying object to destruct"));
            }
            return ObjectManager<ObjType>::get(prhs[0]);
        }

        void clearObjects(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
#ifdef _DEBUG
            mexPrintf("Clearing All Objects.\n");
            mexEvalString("pause(0.2)");
#endif
            ObjectManager<ObjType>::clearObjects();
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

        virtual ~mexInterfaceManager() {
#ifdef _DEBUG
            mexPrintf("Destroying mexInterfaceManager<%s,...>\n", typeid(ObjType).name());
            mexEvalString("pause(0.2)");
#endif
        }

        mexInterfaceManager() {
            using namespace std::placeholders;

            addFunction("new", std::bind(&mexInterfaceManager::new_object, this, _1, _2, _3, _4));  //add 'new' command
            addFunction("delete", std::bind(&mexInterfaceManager::delete_object, this, _1, _2, _3, _4));  //add 'delete' command
            addFunction("clear_mex_objects", std::bind(&mexInterfaceManager::clearObjects, this, _1, _2, _3, _4));  //add 'clear_mex_objects' command
            addFunction("getMethodNames", std::bind(&mexInterfaceManager::getMethodNames, this, _1, _2, _3, _4)); //return cellstr specifying method names

        }

        void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
            /// validate first arg
            if (nrhs < 1 || !mxIsChar(prhs[0])) {
                mexErrMsgIdAndTxt("mexInterfaceManager:argumentError", "Invalid argument: missing method name.");
            }
            std::string funcName;
            try {
                funcName = extras::cmex::getstring(prhs[0]);
#ifdef DAN_DEBUG
                mexPrintf("mexInterfaceManager<%s>::mexFunction\n", typeid(ObjType).name());
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
                    (search->second)(nlhs, plhs, nrhs - 1, &(prhs[1])); //execute command
                }
                else {
                    throw(extras::stacktrace_error(
                        std::string("'") + funcName + std::string("' method not found.")
                    ));
                }
            }
            catch (...) {
                handle_exception(std::current_exception(), funcName);
            }
        }
    };

}}