% [z,varz,nItr,lastNewtonStep,lastR2,final_dR2frac] = splineroot(v,pp,dpp,TOL,minStep,maxItr,min_dR2frac)
% Simultaneously solve the problem:
% z_m = argmin( Sum_i(Sp_i(z)-V_mi))
% 
% Input:
%         v: Nx1 array of values to best fit
%			Note, if all values of v are NaN, the output is NaN
%			otherwise, dimesions where v(d)==NaN are ignored
%         pp: N-dimensional spline structure
%         dpp: (optional) pre-calculated derivative of pp
%               pass dpp=[] to skip.
%         TOL: (default=0.001) Algorithm stops when
%                               (Sum_i(Sp_i(z)-V_mi)^2)<=TOL^2
%         minStep: (default=20*eps) scalar
%            if the computed newton step is smaller than minStep the 
%            algorithm returns
%		  maxItr: (default=10000) maximum number of newton step 
%			 iterations to run
%		  min_dR2frac: (default = 0.0001) minimim fractional difference
%		     between succesive iterations.
%				dR2frac = |R2(i) - R2(i-1)|/R2(i-1)
%					where R2 is calcualted as (Sum_i(Sp_i(z)-V_mi)^2)
%			if there is not sufficient change in R2, the algorithm returns
%
% Outputs:
%	z: the best fit solution to v=pp(z)
%	varz: estimate of fit error (returns StdErr^2)
%	nItr: number of iterations completed
%	lastNewtonStep: value of the last multiplier used in Gauss-newton loop (low value indicates algorithm was not stepping very far)
%	lastR2: last value of the average residual
%	final_dR2frac: final value of the change in residual between succesive steps
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.
% THIS IS A STUB TO A MEX FILE