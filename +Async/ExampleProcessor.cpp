/*
    Example AsyncProcessor Object

    This mex file creates a dummy processor which simply copies the task inputs
    provided via pushTask(Arg1,Arg2,...) to the results queue
    yielding a result
        [Arg1,Arg2,...] = popResult();
*/

#include <mex.h>
#include <extras/async/AsyncProcessor.hpp>

#include <extras/SessionManager/ObjectManager.h> // Object manager includes
#include <extras/SessionManager/mexDispatch.h>

class ExampleProcessor: public extras::async::AsyncProcessor{
protected:
    /// method for Processing Tasks in the task list
    virtual extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& args){
        extras::cmex::mxArrayGroup out = args;
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //let some time pass
        return out;
    }
};

extras::SessionManager::ObjectManager<ExampleProcessor> manager; //global object manager, template type should be changed to your class implementation

//////////////////////////////
// Required NEW function
//
// NOTE: Change this function

//Create New instance
MEX_DEFINE(new) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    int64_t val = manager.create(new ExampleProcessor); //CHANGE THIS LINE

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

	if (err == nullptr) { //no errors, return empty
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
		return;
	}

	//convert exception ptr to struct
	try {
		rethrow_exception(err);
	}
	catch (const std::exception& e) {
		const char* fields[] = { "identifier","message" };
		mxArray* out = mxCreateStructMatrix(1, 1, 2, fields);
		mxSetField(out, 0, "identifier", mxCreateString("ProcessingError"));
		mxSetField(out, 0, "message", mxCreateString(e.what()));

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

/////////////////////
// END of Code, assemble mex function below

// Implement mexFunction using dispatch
MEX_DISPATCH
