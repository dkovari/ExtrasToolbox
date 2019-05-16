#include <mex.h>
#include "MatlabTime.hpp"


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){

    double dn = extras::matlab_now();
    plhs[0] = mxCreateDoubleScalar(dn);
    mexPrintf("Datenum: %g\n",dn);
    extras::DateCE dce(dn);

    mexPrintf("Time: %d-%02d-%02d %02d:%02d:%02.3f\n",dce.year(),dce.month(),dce.mday(),dce.hour(),dce.min(),dce.sec());
}
