/*
testArrays.cpp
MEX function for testing MxObjects, NativeArrays, and mex::NumericArrays

build from MATLAB using command
    >> mex 'testArrays.cpp'
*/

#include <mex.h>
#include "extras/cmex/NumericArray.hpp"
#include "extras/Array.hpp"

/// MATLAB Function Entry Point
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray* prhs[]){
    using namespace extras;
    using namespace extras::cmex;
    if(nrhs<1){
        throw(std::runtime_error("min 1 argument required"));
    }
    mexPrintf("create Array 1:\n");
    mexEvalString("pause(0.1);");
    NumericArray<double> array1(prhs[0]);
    disp(array1);

    mexPrintf("create Native Array\n");
    mexEvalString("pause(0.1);");

    Array<int> NatArray(array1);

    mexPrintf("NativeArray numel: %d\n",NatArray.numel());
    mexPrintf("mexArray[1]=%g, NatArray[1]=%g\n",array1[1],(float)NatArray[1]);

    mexPrintf("NatArray\n");
    NatArray.disp();

    try{
        mexPrintf("NatArray[10]: %g\n",NatArray[10]);
    }catch(std::exception& e){
        mexPrintf("Error!!!\n");
        mexPrintf("\t What(): %s\n",e.what());
    }


    mexPrintf("create outarray\n");
    mexEvalString("pause(0.1);");
    NumericArray<int16_t> outarray(NatArray);

    mexPrintf("outarray[0]=%g\n",(float)outarray[0]);
    mexPrintf("press a key to return\n");
    mexEvalString("pause();");

    plhs[0] = outarray;
}
