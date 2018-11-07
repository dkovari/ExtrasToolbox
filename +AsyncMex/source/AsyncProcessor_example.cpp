/*
Example AsyncProcessor Mex interface
*/

#include "mexAsyncProcessor.h"
#include "ObjectManager.h"
#include "mexDispatch.h"

ObjectManager<mex::AsyncProcessor> manager; //global object manager, template type should be changed to your class implementation

//////////////////////////////
// Required NEW function
//
// NOTE: Change this function

//Create New instance
MEX_DEFINE(new) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    int64_t val = manager.create(new mex::AsyncProcessor); //CHANGE THIS LINE

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

	if (err == nullptr) { //no errors
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
		mxSetField(out, 1, "identifier", mxCreateString("ProcessingError"));
		mxSetField(out, 1, "message", mxCreateString(e.what()));

		plhs[0] = out;
	}
}

/////////////////////
// END of Code, assemble mex function below

// Implement mexFunction using dispatch
MEX_DISPATCH
