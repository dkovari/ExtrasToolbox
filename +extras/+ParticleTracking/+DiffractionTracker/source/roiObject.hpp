#pragma once

#include <extras/Array.hpp>
#include <memory>
#include <extras/cmex/mxobject.hpp>
#include <extras/cmex/NumericArray.hpp>
#include "../../radialcenter/source/radialcenter_mex.hpp" //radialcenter code


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
	std::shared_ptr<extras::cmex::MxObject> roiStruct; //original struct array object used to set roiList


	void setFromStruct(const mxArray* srcStruct) {
		auto newStruct = std::make_shared<extras::cmex::MxObject>(srcStruct);
		if (!newStruct->isstruct()) {
			throw(std::runtime_error("roiAgregator::setFromStruct(): MxObject is not a struct"));
		}
		if (!extras::cmex::hasField(*newStruct, "Window")) {
			throw(std::runtime_error("roiAgregator::setFromStruct(): struct does not contain 'Window' field"));
		}
		if (!extras::cmex::hasField(*newStruct, "UUID")) {
			throw(std::runtime_error("roiAgregator::setFromStruct(): struct does not contain 'UUID' field"));
		}

		size_t len = newStruct->numel();
		//create temporary array that will be pushed into main variables at the end of the function
		// if an error is thrown, original arrays will remain unchanged.
		std::shared_ptr<extras::ArrayBase<double>> newWIND = std::make_shared<extras::Array<double>>(len, 4);
		std::shared_ptr<extras::ArrayBase<double>> newXYc = XYc;
		std::shared_ptr<extras::ArrayBase<double>> newGP = GP;
		std::shared_ptr<extras::ArrayBase<double>> newRadiusFilter = RadiusFilter;

		if (extras::cmex::hasField(*newStruct, "XYc")) {
			newXYc->resize(len, 2);
		}
		else {
			newXYc->resize(0, 2);
		}

		if (extras::cmex::hasField(*newStruct, "GP")) {
			newGP->resize(len);
		}
		else {
			newGP->resize(0,1);
		}
		if (extras::cmex::hasField(*newStruct, "RadiusFilter")) {
			newRadiusFilter->resize(len);
		}
		else {
			newRadiusFilter->resize(0, 1);
		}

		for (size_t n = 0; n < len; ++n) {
			// set window
			extras::cmex::NumericArray<double> Window(mxGetField(*newStruct,n,"Window"));
			(*newWIND)(n, 0) = Window[0]; //X1
			(*newWIND)(n, 1) = Window[0]+Window[2]-1; //X2=x+w-1
			(*newWIND)(n, 2) = Window[1]; //Y1
			(*newWIND)(n, 3) = Window[1] + Window[3] - 1; //Y2=y+h-1

			if (extras::cmex::hasField(*newStruct, "XYc")) {
				extras::cmex::NumericArray<double> sXYc(mxGetField(*newStruct, n, "XYc"));
				(*newXYc)(n, 0) = sXYc[0];
				(*newXYc)(n, 1) = sXYc[1];
			}

			if (extras::cmex::hasField(*newStruct, "GP")) {
				extras::cmex::NumericArray<double> sGP(mxGetField(*newStruct, n, "GP"));
				(*newGP)(n) = sGP[0];
			}
			if (extras::cmex::hasField(*newStruct, "RadiusFilter")) {
				extras::cmex::NumericArray<double> sRadiusFilter(mxGetField(*newStruct, n, "RadiusFilter"));
				(*newRadiusFilter)(n) = sRadiusFilter[0];
			}
		}

		// no errors!
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
};