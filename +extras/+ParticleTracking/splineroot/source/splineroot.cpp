/* SPLINEROOT
[z,varz] = splineroot(v,pp,dpp,TOL,minStep,maxItr)

Simultaneously solve the problem:
z_m = argmin( Sum_i(Sp_i(z)-V_mi))

Input:
v: Nx1 array of values to best fit
pp: N-dimensional spline structure
dpp: (optional) pre-calculated derivative of pp
TOL: (default=0.001) Algorithm stop when (Sum_i(Sp_i(z)-V_mi)^2)<=TOL
minStep:(default=20*eps) scalar
if the computed newton step is smaller than minStep the algorithm returns
maxItr: (default=10000) max num of iterations
minR2frac: (default=0.00001) min fractional difference in R2_N between successive iterations
MaxR2: (default=Inf) max initial R2, if R2 of val vs all knots in the spline is greater than MaxR2, the algorithm returns NaN

/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/

#include "splineroot_mex.hpp"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    extras::ParticleTracking::splineroot_mex(nlhs, plhs, nrhs, prhs);
}
