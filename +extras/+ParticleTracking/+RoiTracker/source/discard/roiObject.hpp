#pragma once

#include <extras/Array.hpp>
#include <memory>
#include <extras/cmex/MxStruct.hpp>
#include <extras/cmex/NumericArray.hpp>
#include "../../radialcenter/source/radialcenter_mex.hpp" //radialcenter code


typedef std::shared_ptr<extras::cmex::MxStruct> MxStruct_Ptr;

/// enum specifying xy localization algorithm
enum XY_FUNCTION{
    BARYCENTER,
    RADIALCENTER
};

const char* xyFunctionName(XY_FUNCTION xyF) {
	switch (xyF) {
	case BARYCENTER:
		return "barycenter";
	case RADIALCENTER:
		return "radialcenter";
	default:
		throw(std::runtime_error("BAD Y_FUNCTION"));
	}

}

XY_FUNCTION xyFunction(const char* name) {
	if (strcmpi(name, "barycenter")==0) {
		return BARYCENTER;
	}
	else if (strcmpi(name, "radialcenter") == 0) {
		return RADIALCENTER;
	}
	else {
		throw(std::runtime_error("xyFunction() string not valid XY_FUNCTION name"));
	}
}

// List of roiObjects (see MATLAB packaget extras.roi)
//
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
struct roiAgregator {

	std::shared_ptr<extras::ArrayBase<double>> WIND = std::make_shared<extras::Array<double>>(); //array containing windows of the rois, formatted as [X1,X2,Y1,Y2]
	std::shared_ptr<extras::ArrayBase<double>> XYc=std::make_shared<extras::Array<double>>(); //optional array containing XYc centroid guesses
	std::shared_ptr<extras::ArrayBase<double>> GP = std::make_shared<extras::Array<double>>(); //optional array containing GP for radialcenter
	std::shared_ptr<extras::ArrayBase<double>> RadiusFilter = std::make_shared<extras::Array<double>>(); //optional array containing RadiusFilter for radialcenter
	MxStruct_Ptr roiStruct = std::make_shared<extras::cmex::MxStruct>(); //original struct array object used to set roiList

	void setFromStruct(const mxArray* srcStruct) {
		using namespace extras::cmex;
		auto newStruct = std::make_shared<MxStruct>(srcStruct);

		if(!newStruct->isfield("Window")){
			throw(std::runtime_error("roiAgregator::setFromStruct(): struct does not contain 'Window' field"));
		}

		size_t len = newStruct->numel(); //number of entries in roiList

		// Set Windows
		std::shared_ptr<extras::ArrayBase<double>> newWIND = std::make_shared<extras::Array<double>>(len, 4);
		for (size_t n = 0; n < len; ++n) {
			// set window
			NumericArray<double> Window = MxObject((*newStruct)(n,"Window"));
			(*newWIND)(n, 0) = Window[0]; //X1
			(*newWIND)(n, 1) = Window[0]+Window[2]-1; //X2=x+w-1
			(*newWIND)(n, 2) = Window[1]; //Y1
			(*newWIND)(n, 3) = Window[1] + Window[3] - 1; //Y2=y+h-1
		}

		// set XYc
		std::shared_ptr<extras::ArrayBase<double>> newXYc = XYc;
		if(newStruct->isfield("XYc")){
			newXYc->resize(len, 2);
			for (size_t n = 0; n < len; ++n) {
				// set window
				NumericArray<double> sXYc = MxObject((*newStruct)(n,"XYc"));
				(*newXYc)(n,0) = sXYc[0];
				(*newXYc)(n,1) = sXYc[1];
			}
		}

		// Set GP
		std::shared_ptr<extras::ArrayBase<double>> newGP = GP;
		if(newStruct->isfield("GP")){
			newGP->resize(len, 1);
			for (size_t n = 0; n < len; ++n) {
				// set window
				NumericArray<double> sGP = MxObject((*newStruct)(n,"GP"));
				(*newGP)(n,0) = sGP[0];
			}
		}

		// Set RadiusFilter
		std::shared_ptr<extras::ArrayBase<double>> newRadiusFilter = RadiusFilter;
		if(newStruct->isfield("RadiusFilter")){
			newGP->resize(len, 1);
			for (size_t n = 0; n < len; ++n) {
				// set window
				NumericArray<double> sRadiusFilter = MxObject((*newStruct)(n,"RadiusFilter"));
				(*newRadiusFilter)(n,0) = sRadiusFilter[0];
			}
		}

		// no errors so far!
		// set main variables
		newStruct->makePersistent();
		roiStruct = newStruct;
		WIND = newWIND;
		GP = newGP;
		XYc = newXYc;
		RadiusFilter = newRadiusFilter;

	}

	mxArray* getCopyOfRoiStruct() const{
		return mxDuplicateArray(*roiStruct);
	}

	size_t numberOfROI() const {
		return mxGetNumberOfElements(*roiStruct);
	}

	/// globals for radial center
	rcdefs::COM_METHOD COMmethod = rcdefs::MEAN_ABS;
	double DistanceFactor = INFINITY;

	/// globals for barycenter
    double barycenterLimFrac = 0.2; ///< LimFrac used by barycenter

	XY_FUNCTION xyMethod = RADIALCENTER;
};
