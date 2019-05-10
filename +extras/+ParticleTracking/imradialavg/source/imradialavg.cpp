/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#include "imradialavg_mex.hpp"

/** Callable MEX function
* [Avg,BinLocations,BinCounts] = imradialavg(I,x0,y0,Rmax,Rmin,BinWidth)
* Computer azmuthal average of image around specified location
*Inputs:
*   I: the image to use (should not be complex, but any other numeric type
*       is fine)
*   x0,y0: scalar numbers specifying the coordinates
*           (NOTE: <1,1> is top left corner of image)
*   Rmax(=NaN): scalar specifying maximum radius (NaN indicated image edges
*       are the limits)
*   Rmin(=0): minimum radius to use
*       NOTE: If you specify both Rmax and Rmin you can use the more
*       logical ordering: imradialavg(__,Rmin,Rmax);
*   BinWidth(=1): width and spacing of the bins
*
* Outputs:
*   Avg: radial averages
*   BinLocations: locations of the radial bins (e.g. 0,1,...,Rmax)
*   BinCounts: number of pixels accumulated into each bin
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	extras::ParticleTracking::imradialavg_mex(nlhs, plhs, nrhs, prhs);
}
