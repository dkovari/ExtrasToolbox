/* Expected Syntax
% setPersistentArgs(Name,Value)
%
% Parameters
% ----------------------------------------------------------------------
%   'ROIstruct',struct
%       struct array specifying ROI information
%       number of elements corresponds with number of particles/windows to track
%       Fields:
%           .UUID = '...' char array specifying uniquie identifier for
%                         ROI
%           .Window = [x,y,w,h] roi window
%           .RadiusFilter = ## size of radius filter to use for radial
%                              center
%           .MinRadius = ## minimum radius (in pixels) to use when
%                           creating the radial average
%           .MaxRadius = ## maximum radius (in pixels) to use when
%                           creating the radial average
%           .BinWidth = ## (default = 1) bin width (in pixels) for
%                          radial average
%           .ReferenceUUID = '...' or {'...','...',...} uuid of
%                            particles to use as reference
%           .IsCalibrated = t/f flag if particle has a look-up-table
%           .Zspline = pp spline describing LUT, used by splineroot
%           .dZspline = dpp derivative of Zspline, used by splineroot
% ----------------------------------------------------------------------
% Global Parameters:
%   'COMmethod', ''
%   'DistanceFactor'
%   'splineroot_TOL'
%   'splineroot_minStep'
%   'splineroot_maxItr'
%   'splineroot_min_dR2frac'
*/

#include "roiTracker.hpp"

extras::SessionManager::ObjectManager<roiTrackerXY<>> manager;
extras::async::PersistentArgsProcessorInterface<roiTrackerXY<>, manager> mex_interface; //create interface manager for the processor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	mex_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}
