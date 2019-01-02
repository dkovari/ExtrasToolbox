#include <mex.h>
#include "extras/cmex/mxClassIDhelpers.hpp"

#include "extras/cmex/MxStruct.hpp"

/*
Display the properties of mxArray data
*/

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    for(size_t n=0;n<nrhs;++n){

        mexPrintf("Arg%d:\n",n+1);
            mexPrintf("\tClass Type: %s\n",extras::cmex::classname(mxGetClassID(prhs[n])));
            mexPrintf("\tclassname: %s\n",mxGetClassName(prhs[n]));
            mexPrintf("\tElementSize: %d\n",mxGetElementSize(prhs[n]));
            mexPrintf("\tIsComplex: %d\n",mxIsComplex(prhs[n]));
            mexPrintf("\tDimensions: [");
                mexPrintf("%d",mxGetDimensions(prhs[n])[0]);
                for(size_t j=1;j<mxGetNumberOfDimensions(prhs[n]);++j){
                    mexPrintf(" x %d",mxGetDimensions(prhs[n])[j]);
                }
                mexPrintf("]\n");
            if(mxIsNumeric(prhs[n])){
                if(!mxIsComplex(prhs[n])){
                    mexPrintf("\tDataPtr: %p",mxGetData(prhs[n]));
                }else{ //complex
                    #if MX_HAS_INTERLEAVED_COMPLEX
                        mexPrintf("\tInterleavedPtr: %p\n",mxGetData(prhs[n]));
                    #else
                        mexPrintf("\t RealPtr: %p,  ImagPtr: %p\n",mxGetData(prhs[n]),mxGetImagData(prhs[n]));
                    #endif
                }
            }else{
                mexPrintf("\tNon-Numeric Ptr: %p\n",mxGetData(prhs[n]));
            }
            if(mxIsStruct(prhs[n])){
                mexPrintf("\tNumFields: %d\n",mxGetNumberOfFields(prhs[n]));
                for(size_t j=0;j<mxGetNumberOfFields(prhs[n]);++j){
                    mexPrintf("\t\t%s\n",mxGetFieldNameByNumber(prhs[n],j));
                }
            }

        mexPrintf("\n");

    }

    mexPrintf("creating struct\n");
    const char* f="f1";
    mxArray* s = mxCreateStructMatrix(1, 1, 1, &f);
    mexPrintf("f1: %p\n",mxGetFieldByNumber(s,0,0));
    //mxSetFieldByNumber(s, 0, 0, mxCreateDoubleScalar(10));
    //mexPrintf("f1: %p\n",mxGetFieldByNumber(s,0,0));
    mxAddField(s,"t1");
    mexPrintf("t1: %p\n",mxGetFieldByNumber(s,0,1));

    extras::cmex::MxStruct Sr;
    Sr.reshape(1,1);
    Sr(0,"test") = mxCreateDoubleScalar(100);

    plhs[0] = Sr;
}
