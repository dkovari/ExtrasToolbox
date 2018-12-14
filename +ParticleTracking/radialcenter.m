% [x,y,varXY,d2] = radialcenter(I,WIND,GP)
%                = radialcenter(__,name,value);
%
% Estimate the center of radial symmetry of an image
%
% Input:
%   I: the image to process
%   WIND: [N x 4] specifying windows [X1,X2,Y1,Y2], default is entire image
%   GP (default=5): optional exponent factor to use for magnitude weighting
%       GP must be either scalar, or numel(GP)==size(WIND,1)
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
%   'RadiusFilter',val or [v1,v2,...vN]
%   'XYc',[X,Y] : particle center estimates
%   'COMmethod',method
%       method='meanABS' : use COM on |I-mean(I)| to estimate center for radius filter
%       method='normal': use COM on unmodified I to estimate center
%       method='gradmag': use magnitude of image gradient to find COM
%   'DistanceFactor',value
%       Rate to use in logistic function defining the filter window around xc,yc
%       Default value is Inf, which indicates the Hat-function:
%           W=double(r<Radius) is used instead of a logistic function.
%       If RadiusFilter==0 DistanceFactor is the exponent of the inverse
%       distance function use to weight the pixels:
%           w = w/r^DF
%       where r is the distance of a pixel from the extimated com or the
%       specified XYc(n,:) coordinate

%This file is a stub for a MEX function
