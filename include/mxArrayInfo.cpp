#include <mex.h>
#include "extras/cmex/mxClassIDhelpers.hpp"

#include "extras/cmex/MxStruct.hpp"
#include "extras/cmex/MxCellArray.hpp"

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
    mexPrintf("Sr.numel(): %d\n",Sr.numel());
    Sr.reshape(1,1);
    Sr(0,"test") = mxCreateDoubleScalar(100);

    plhs[0] = Sr;


    /*/////////////
    // CELL

    mxArray* c1 = mxCreateCellMatrix(1, 0);
    //mxSetCell(c1,0,mxCreateString("c1{1}"));
    mexCallMATLAB(0,NULL,1,&c1,"disp");
    mexPrintf("press a key\n");
    mexEvalString("pause");

    size_t dims[] = {3,1};
    mxSetDimensions(c1, dims, 2);
    mexPrintf("press a key\n");
    mexEvalString("pause");
    mxSetCell(c1,1,mxCreateString("c1{2}"));
    mexCallMATLAB(0,NULL,1,&c1,"disp");
    mexPrintf("press a key\n");
    mexEvalString("pause");


    mexPrintf("done with c1\n");*/


    if(nlhs>1){
        extras::cmex::MxCellArray C;
        C.reshape(5,1);
        C(0) = "fadsfasdf";
        //C(1) = "test2";
        plhs[1] = C;
    }
}
