/*
    radialcenterAsync Object

    This mex file creates a processor which applies the radialcenter routine asynchrounously.

    Usage:

    >> rcA = radialcenterAsync(); %create async processor
    >> rcA.pushTask(IMAGE,...Other Args...); %process IMAGE using specified arguments
    >> ... %do other things and wait for processor
    >> [x,y,varXY,d2] = popResults();
    >> rcA.setPersistentArgs(WIND,GP); %sets persistent arguments
    >> rcA.pushTask(IMAGE2); %push IMAGE and persistent WIND,GP
    >> rcA.pushTask(IMAGE3); %push another image, also using persistent Args
    >> rcA.pushTask(IMAGE4,WIND4,GP4); %causes error since persisten arguements are set
*/

#include <extras/cmex/NumericArray.hpp>

#include <extras/async/ProcessorPersistentArgs.hpp>
//#include <extras/async/AsyncProcessor.hpp>
#include "source/radialcenter_mex.hpp"

#include <extras/SessionManager/ObjectManager.h> // Object manager includes
#include <extras/SessionManager/mexDispatch.h>

class rcProc:public extras::async::ProcessorWithPersistentArgs{//extras::async::AsyncProcessor{//
protected:
    /// method for Processing Tasks in the task list
    virtual extras::cmex::mxArrayGroup ProcessTask(const std::pair<extras::cmex::mxArrayGroup,std::shared_ptr<extras::cmex::mxArrayGroup>>& args){
        using namespace std;
        extras::cmex::mxArrayGroup out(4);

		// DEBUG
		/*throw(runtime_error(string("in ProcessTask  ")+
			string("nTaskArgs: ") + to_string(args.first.size())+string(" ParamArgs:")+to_string(args.second->size())
		));*/

        const mxArray* *pA;
        size_t sz = args.first.size()+args.second->size();
        pA = new const mxArray* [sz];

        for(size_t n=0;n<args.first.size();++n){
            try{
                pA[n] = args.first.getArray(n);
            }catch(const exception& e){
                throw(runtime_error(
                    string(e.what())+string(" sz:")+to_string(sz)+string(" Arg1: ")+to_string(n)
                ));
            }
        }
        for(size_t n=0;n<args.second->size();++n){
            try{
                pA[n+args.first.size()] = args.second->getArray(n);
            }catch(const exception& e){
                throw(runtime_error(
                    string(e.what())+string(" sz:")+to_string(sz)+string(" Arg2: ")+to_string(n)
                ));
            }
        }

        extras::ParticleTracking::radialcenter_mex(4, out, sz, pA);

        delete[] pA;

        return out;
    }
};



extras::SessionManager::ObjectManager<rcProc> manager; //global object manager, template type should be changed to your class implementation

//////////////////////////////
// Required NEW function
//
// NOTE: Change this function

//Create New instance
MEX_DEFINE(new) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    int64_t val = manager.create(new rcProc); //CHANGE THIS LINE

    plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
    *(((int64_t*)mxGetData(plhs[0]))) = val;
}

////////////////////////////////
// Required Standard interface functions

//delete instance
MEX_DEFINE(delete) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object to destruct"));
    }
    manager.destroy(prhs[0]);
}

// Class Methods
//==============================

//remainingTasks
MEX_DEFINE(remainingTasks) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    plhs[0] = mxCreateDoubleScalar(manager.get(prhs[0])->remainingTasks());
}

//availableResults
MEX_DEFINE(availableResults) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    plhs[0] = mxCreateDoubleScalar(manager.get(prhs[0])->availableResults());
}

//running
MEX_DEFINE(running) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    plhs[0] = mxCreateLogicalScalar(manager.get(prhs[0])->running());
}

//cancelRemainingTasks
MEX_DEFINE(cancelRemainingTasks) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    manager.get(prhs[0])->cancelRemainingTasks();
}

//pushTask
MEX_DEFINE(pushTask) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    manager.get(prhs[0])->pushTask(nrhs-1,&(prhs[1]));
}

//numResultOutputArgs
MEX_DEFINE(numResultOutputArgs)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    plhs[0] = mxCreateDoubleScalar(manager.get(prhs[0])->numResultOutputArgs());
}

//popResult
MEX_DEFINE(popResult) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    manager.get(prhs[0])->popResult().copyTo(nlhs,plhs);
}

//clearResults
MEX_DEFINE(clearResults) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    manager.get(prhs[0])->clearResults();
}

//pauseProcessor
MEX_DEFINE(pause)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    manager.get(prhs[0])->pause();
}

//resumeProcessor
MEX_DEFINE(resume)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    manager.get(prhs[0])->resume();
}

//wasErrorThrown
MEX_DEFINE(wasErrorThrown)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    plhs[0] = mxCreateLogicalScalar(manager.get(prhs[0])->wasErrorThrown());
}

//checkError
MEX_DEFINE(getError)(int nlhs, mxArray* plhs[],
	int nrhs, const mxArray* prhs[])
{
	if (nrhs < 1) {
		throw(std::runtime_error("requires intptr argument specifying mtdat object to close"));
	}

	std::exception_ptr err = manager.get(prhs[0])->getError();

    mexPrintf("In getError\n");
    mexEvalString("pause(0.2)");

	if (err == nullptr) { //no errors, return empty
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
		return;
	}



	//convert exception ptr to struct
	try {
		rethrow_exception(err);
	}
	catch (const std::exception& e) {

        mexPrintf("error: what: %s\n",e.what());
        mexEvalString("pause(0.2)");


		const char* fields[] = { "identifier","message" };
		mxArray* out = mxCreateStructMatrix(1, 1, 2, fields);
		mxSetFieldByNumber(out, 0, 0, mxCreateString("ProcessingError"));
		mxSetFieldByNumber(out, 0, 1, mxCreateString(e.what()));

		plhs[0] = out;
	}
}

//clearError
MEX_DEFINE(clearError)(int nlhs, mxArray* plhs[],
	int nrhs, const mxArray* prhs[])
{
	if (nrhs < 1) {
		throw(std::runtime_error("requires intptr argument specifying mtdat object to close"));
	}
	manager.get(prhs[0])->clearError();
}


//setPersistentArgs
MEX_DEFINE(setPersistentArgs) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    manager.get(prhs[0])->setPersistentArgs(nrhs-1,&(prhs[1]));
}

//clear Persistent Args
MEX_DEFINE(clearPersistentArgs) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }
    manager.get(prhs[0])->clearPersistentArgs();
}


/////////////////////
// END of Code, assemble mex function below

// Implement mexFunction using dispatch
MEX_DISPATCH
