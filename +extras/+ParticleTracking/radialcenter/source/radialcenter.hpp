/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <extras/Array.hpp> //include extras::extras::Array, be sure to add +extras/include to your include path

#include <math.h>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>
#include <extras/assert.hpp>

#include "radialcenter.h"


namespace extras{namespace ParticleTracking{

	struct RadialcenterParameters_Shared{
		std::shared_ptr<extras::ArrayBase<double>> WIND = std::make_shared<extras::Array<double>>();
		std::shared_ptr<extras::ArrayBase<double>> RadiusCutoff = std::make_shared<extras::Array<double>>(std::vector<double>({ INFINITY })); //fringe pattern max radius
		std::shared_ptr<extras::ArrayBase<double>> CutoffFactor = std::make_shared<extras::Array<double>>(std::vector<double>({ INFINITY })); //fringe pattern max radius cutoff factor
		std::shared_ptr<extras::ArrayBase<double>> XYc = std::make_shared<extras::Array<double>>(); //initial centroid guess, initialize as empty array
		rcdefs::COM_METHOD COMmethod = rcdefs::GRAD_MAG; //method used for estimating center of mass
		std::shared_ptr<extras::ArrayBase<double>> DistanceExponent = std::make_shared<extras::Array<double>>(std::vector<double>({ 1 })); //Distance-depencence exponent
		std::shared_ptr<extras::ArrayBase<double>> GradientExponent = std::make_shared<extras::Array<double>>(std::vector<double>({ 5 })); //gradient exponent

	};

	///Radial Center Detection
	///Description:
	/// radialcenter uses Parthasarathy's radial symmetry algorithm to detect origins of asmuthal symmetry in an image
	/// This implementation is capable of using sub-windows to look for multiple particles in the same image
	///
	///Usage:
	/// This function is a template allowing different output types to be specified.
	///
	/// Output argument order is:
	///	X - the x locations of the particle centers
	///	Y - the y locations of the particle centers
	///	varXY - [Nx2] extras::Array of the localization confidence level for X (column 1) and Y (column 2)
	///	RWR/N - the weighted-average distance between the estimated center and all the graident vectors in the sub-image
	template<class OutContainerClass=extras::Array<double>,typename ImageType=double> //OutContainerClass should be class derived from extras::ArrayBase
	std::vector<OutContainerClass> radialcenter(const extras::ArrayBase<ImageType>& I, //input image
												const RadialcenterParameters_Shared& params = RadialcenterParameters_Shared())//parameters
	{
		// Check Input Dimensions and Parameters
		//---------------------------------------
		using namespace std;

		// Validate WIND
		if(!params.WIND->isempty() && params.WIND->nCols() != 4){
			throw(runtime_error("radialcenter: WIND must be empty or Mx4 extras::Array."));
		}
		size_t nPart = max(size_t(1), params.WIND->nRows());//number of particles to find

		//validate XYc
		if(!params.XYc->isempty()){
			if(params.XYc->nCols()!=2){
				throw(std::runtime_error("radialcenter: XYc must have two columns"));
			}
			if(!params.WIND->isempty()){
				if(params.XYc->nRows()!=params.WIND->nRows()){
					throw(std::runtime_error("radialcenter: nRows XYc must match nRows WIND"));
				}
			}
			else{
				//nPart_via_WIND = false;
				nPart = params.XYc->nRows();
			}
		}

		//Create Params Struct
		RadialcenterParameters rc_params;
		rc_params.COMmethod = params.COMmethod;
		rc_params.nRadiusCutoff = params.RadiusCutoff->numel();
		rc_params.RadiusCutoff = params.RadiusCutoff->getdata();
		rc_params.nCutoffFactor= params.CutoffFactor->numel();
		rc_params.CutoffFactor = params.CutoffFactor->getdata();
		rc_params.nDistanceExponent= params.DistanceExponent->numel();
		rc_params.DistanceExponent = params.DistanceExponent->getdata();
		rc_params.nGradientExponent= params.GradientExponent->numel();
		rc_params.GradientExponent = params.GradientExponent->getdata();
		rc_params.nWIND = params.WIND->nRows();
		rc_params.WIND = params.WIND->getdata();
		rc_params.nXYc = params.XYc->nRows();
		rc_params.XYc = params.XYc->getdata();

		//Setup Output variables
		//----------------------------
		vector<OutContainerClass> out;
		out.resize(4);

		auto& x = out[0];
		auto& y = out[1];
		auto& varXY = out[2];
		auto& RWR_N = out[3];

		// Resize output vars
		x.resize(nPart, 1);
		y.resize(nPart, 1);

		varXY.resize(nPart, 2);
		RWR_N.resize(nPart, 1);

		//Call radialcenter
		//---------------------
		radialcenter(x.getdata(),y.getdata(),varXY.getdata(),RWR_N.getdata(),
					I.getdata(),I.nRows(),I.nCols(),
					rc_params);

		// Return output
		return out;
	}
}}
