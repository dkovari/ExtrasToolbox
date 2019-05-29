classdef RoiTracker3D < extras.ParticleTracking.RoiTracker.RoiTracker
% Asynchronous Radialcenter image processor with 3D tracking via splineroot
% Extends extras...RoiTracker to also use splineroot() LUT-based tracking
%
% RoiTracker3D uses the LUT list included inside roiObject3D ROIs to
% determine the depth-tracking parameters.
%
% To set the roi info call:
%   setParameters('roiList',toStruct(YourArray_of_roiObjects3d));
%
% Results are stored in struct (see RoiTracker)
%    ResultData. ...
%              ...
%              .roiList(n).
%                       ...
%                       .RadialAverage: numeric array holding radial
%                                       profile around centroid
%                       .RadialAverage_rloc: location of the radial profile
%                                            data points
%                       .LUT(m).
%                              ...
%                              .DepthResult.
%                                   .Z -> calculated position in lut
%                                   .varZ -> statistical variance of Z
%                                   .nItr -> number of iterations
%                                   .s -> last newton step size
%                                   .R2 -> last sq. residual
%       							.dR2frac -> fractional change in sq residual at last step
%           						.initR2 -> initial sq. residual from initial nearest knot guess
%
% Dependencies:
%   Relies on extras.ParticleTracking.RoiTracker.RoiTracker3D_mex
%   See ...\+extras\+ParticleTracking\+RoiTracker\source for c++ code
%% Copyright 2019, Daniel T. Kovari, Emory University
%  All rights reserved.

    %% Create
    methods
        function this = RoiTracker3D()
            this.change_MEX_FUNCTION(@extras.ParticleTracking.RoiTracker.RoiTracker3D_mex);
            this.Name = 'RoiTracker3D'; %Change Name
        end
    end
end
