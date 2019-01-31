//THIS FILE DEFINES THE MANDATORY MEX_DEFINEs FOR AsyncMexProcessor OBJECTS
//INCLUDE THIS FILE AFTER DEFINING Session manager


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
MEX_DEFINE(pauseProcessor)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    manager.get(prhs[0])->pauseProcessor();
}

//resumeProcessor
MEX_DEFINE(resumeProcessor)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    manager.get(prhs[0])->resumeProcessor();
}

//errorThrown
MEX_DEFINE(errorThrown)(int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    if (nrhs < 1) {
        throw(std::runtime_error("requires intptr argument specifying object"));
    }

    plhs[0] = mxCreateLogicalScalar(manager.get(prhs[0])->errorThrown());
}

//checkError
MEX_DEFINE(checkError)(int nlhs, mxArray* plhs[],
	int nrhs, const mxArray* prhs[])
{
	if (nrhs < 1) {
		throw(std::runtime_error("requires intptr argument specifying mtdat object to close"));
	}

	std::exception_ptr err = manager.get(prhs[0])->checkError();

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
