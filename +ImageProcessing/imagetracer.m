function [TraceData,im_flat,im_filt,im_ridge,im_bin] = imagetracer(im,varargin)
% Trace line-like features in a grayscale image
%

%% Parse Inputs
p = inputParser;

addParameter(p,'MinSize',0,@(x) isscalar(x)&&isnumeric(x));
addParameter(p,'MaxSize',Inf,@(x) isscalar(x)&&isnumeric(x));
addParameter(p,'FlatOrder',2,@(x) isscalar(x)&&isnumeric(x));
addParameter(p,'NoiseFilterSize',5,@(x) isscalar(x)&&isnumeric(x));
addParameter(p,'Interactive',false,@isscalar);



