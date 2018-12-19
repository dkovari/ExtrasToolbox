#pragma once

#include <extras/cmex/mexextras.hpp>
#include "ObjectManager.h"
#include <extras/string_extras.hpp>
#include <functional>

namespace extras{namespace SessionManager{
    /// mexFunction Class wrapper
    /// Use this to wrap c++ objects so that their methods can be called from MATLAB
    ///
    /// Usage:
    /// Your mex cpp file should look like this
    ///
    ///     mexInterface<YOUR_CLASS_TYPE> MexInt; //global instance of mexInterface, persists between mexFunction calls
    ///
    ///     void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    ///         MexInt.mexFunction(nlhs,plhs,nrhs,prhs);
    ///     }
    ///
    /// In MATLAB you would call
    ///     >> p_obj = YOUR_MEX_FUNCTION('new'); %get pointer to c++ object
    ///     >> YOUR_MEX_FUNCTION('your_method',p_obj,Arg1,Arg2,...); %run method, note: must have extended mexInterface to implement your_method
    ///     >> YOUR_MEX_FUNCTION('delete',p_obj); %delete associated object
    ///
    /// Extending mexInterface:
    ///  You should extend mexInterface for your object as follows
    ///
    ///     class yourMexInterface: public mexInterface<yourClass>{
    ///         /// implement function named 'your_method'
    ///         void your_method(int nlhs, mxArray** plhs,int nrhs, const mxArray** prhs){
    ///            if (nrhs < 1) {
    ///                 throw(std::runtime_error("requires intptr argument specifying object to destruct"));
    ///             }
    ///             ObjManager.get(prhs[0])->yourMethod(...); //yourClass should have public member function yourMethod(...)
    ///         }
    ///     public:
    ///         yourMexInterface(){
    ///             using namespace std::placeholders;
    ///             addFunction('your_method',std::bind(&yourMexInterface::your_method,*this,_1,_2,_3,_4)); //add your_method to function list
    ///         }
    ///     }
    ///
    template<class ObjType>
    class mexInterface{
    private:
        std::map<std::string, std::function<void(int,mxArray**,int,const mxArray* *)>> functionMap; //maps method name to class function
    protected:

        extras::SessionManager::ObjectManager<ObjType> ObjManager; //Associated Object Manager

        /// implement 'new' function interface
        void new_object(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
            int64_t val = ObjManager.create(new ObjType); //CHANGE THIS LINE

            plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
            *(((int64_t*)mxGetData(plhs[0]))) = val;
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
            functionMap.insert(std::make_pair(name, func));
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
                source+= string("mexInterface<")+string(typeid(ObjManager).name())+string(">:");
    			source += functionName;
    			mexErrMsgIdAndTxt(source.c_str(),"Caught exception: %s", e.what());
    		}
    	}

        /// helper function for getting object instance from mxArray holding int64_ptr
        /// Returns a shared pointer to the object instance
        std::shared_ptr<ObjType> getObjectPtr(int nrhs, const mxArray *prhs[]){
            if (nrhs < 1) {
                throw(std::runtime_error("requires intptr argument specifying object to destruct"));
            }
            return ObjManager.get(prhs[0]);
        }

    public:

        mexInterface(){
            using namespace std::placeholders;
            addFunction("new",std::bind(&mexInterface::new_object,*this,_1,_2,_3,_4));  //add 'new' command
            addFunction("delete",std::bind(&mexInterface::delete_object,*this,_1,_2,_3,_4));  //add 'delete' command
        }

        void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
            /// validate first arg
            if (nrhs < 1 || !mxIsChar(prhs[0])){
                mexErrMsgIdAndTxt("mexInterface:argumentError", "Invalid argument: missing operation.");
            }
            std::string funcName;
            try{
                funcName = extras::cmex::getstring(prhs[0]);

                auto search = functionMap.find(extras::cmex::getstring);
                if (search != functionMap.end()) {
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
