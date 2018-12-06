#include <mex.h>
#include "mxClassIDhelpers.hpp"

/*
Display the properties of mxArray data
*/

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    for(size_t n=0;n<nrhs;++n){

        mexPrintf("Arg%d:\n",n+1);
            mexPrintf("\tClass Type: %s\n",classname(mxGetClassID(prhs[n])));
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
}
