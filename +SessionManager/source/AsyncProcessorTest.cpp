
#include "AsyncMexProcessor.h"
#include "ObjectManager.h"
#include "mexDispatch.h"
#include "AsyncTest.h"

ObjectManager<AsyncTest> manager;

//Create New instance
MEX_DEFINE(new) (int nlhs,mxArray* plhs[],
                int nrhs, const mxArray* prhs[])
{
    int64_t val = manager.create(new AsyncTest);
    plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
    *(((int64_t*)mxGetData(plhs[0]))) = val;
}

#include "AsyncMexDefines.cpp" //include the essential interface functions

// Implement mexFunction using dispatch
MEX_DISPATCH
