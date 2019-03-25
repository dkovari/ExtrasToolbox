/*//////////////////////////////
/// Wrapper for radialcenter, accepting the standard arguments for a mexFunction
/*
% [x,y,varXY,d2] = radialcenter(I,WIND)
%                = radialcenter(__,name,value);
%
% Estimate the center of radial symmetry of an image
%
% Input:
%   I: the image to process
%   WIND: [N x 4] specifying windows [x,y,w,h], default is entire image
%
% Output:
%   x,y: center positions
%
%   varXY: variance estimate of the fit
%       varXY = [Vx,Vy], where Vx and Vy are the variances of each X and Y
%
%   d2: the square of the weighted residual, normalized by the effective number of pixels
%       d2>>1 indicates poor localization. This roughly characterizes the
%       distance between each gradient line and the determined center
%       location.
%
%      In practice, d2 is a good metric for determing if an image has an
%      apparent symmetric center, while varXY is useful for characterizing the
%      precision of the fit
%
%
% Name,Value Parameters:
% -------------------------
%   'RadiusCutoff',val or [v1,v2,...vN]: fringe size cutoff
%	'CutoffFactor',val or [v1,v2,...vN]: size cutoff is applied by wieghting using a logistic function :1/(1 + exp(CutoffFactor*(r_guess - RadiusCutoff)));
%		where r_guess is the estimated center of symmetry (either supplied via XYC or found by Image central moment, aka image "center of mass")
%		default = INFINITY (i.e. top-hat function)
%   'XYc',[X,Y] : particle center estimates
%   'COMmethod',method
%       method='meanABS' : use COM on |I-mean(I)| to estimate center for radius filter
%       method='normal': use COM on unmodified I to estimate center
%       method='gradmag': use magnitude of image gradient to find COM (defalut)
%   'DistanceExponent',value or [v1,v2,...,vN]: distance scaling from center guess Wii *= 1/r_guess^(DistanceExponent)
%	'GradientExponent',value or [v1,v2,...,vN]: gradient scaling from center guess Wii *= |GradI_i|^(DistanceExponent)
*/

/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/


#include "radialcenter_mex.hpp"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    extras::ParticleTracking::radialcenter_mex(nlhs,plhs,nrhs,prhs);
}
