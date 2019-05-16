/*
[X,Y] = BaryCenter_classic(Image, WIND, Sz, LimFrac)
Input:
Image: 2D matrix with image data
WIND: [n x 4] array specifying subwindows to process
if not specified, defaults to entire image
Sz: not used, set to anything
LimFrac: Threshold factor for selecting brightest and darkest regions
default=0.2
Threshold is calculated as:
Low = Range*LimFrac + min
UP = Range*(1-LimFrac) + min
Outputs:
[X,Y] coordinates for each window
if something went wrong returns NaN
*/


#include "barycenter_mex.hpp"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	extras::ParticleTracking::barycenter_mex(nlhs, plhs,nrhs,prhs);
}
