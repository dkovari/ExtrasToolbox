#include "imradialavg_mex.hpp"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	extras::ParticleTracking::imradialavg_mex(nlhs, plhs, nrhs, prhs);
}
