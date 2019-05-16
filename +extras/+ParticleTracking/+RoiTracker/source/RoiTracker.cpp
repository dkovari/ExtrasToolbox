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

/** COMPRESSION INCLUDES & LIBS          ****************************
=====================================================================
This header depends on ZLIB for reading and writing files compressed
with the gz format. Therefore you need to have zlib built/installed
on your system.

A version of ZLIB is included with the ExtrasToolbox located in
.../+extras/external_libs/zlib
Look at that folder for build instructions.
Alternatively, is you are using a *nix-type system, you might have
better luck using your package manager to install zlib.

When building, be sure to include the location of zlib.h and to link to
the compiled zlib-lib files.
************************************************************************/

#include "RoiTracker.hpp"

extras::SessionManager::ObjectManager<extras::ParticleTracking::RoiTracker> manager;
extras::ParticleTracking::RoiTrackerInterface<extras::ParticleTracking::RoiTracker,manager> mex_interface;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	mex_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}