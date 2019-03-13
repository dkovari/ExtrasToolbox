/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <extras/async/ParamProcessor.hpp>
#include <extras/cmex/MxObject.hpp>
#include <extras/Array.hpp>

namespace extras { namespace particletracking {

	class RoiParameterMap : extras::cmex::ParameterMxMap {
	protected:
		extras::Array<double> WIND;
		extras::Array<double> XYc;
		extras::Array<double> GP;
		extras::Array<double> RadiusFilter;
	public:
		RoiParameterMap():extras::cmex::ParameterMxMap(false){
			/// Add MAP Defaults
			(*this)["xyMethod"].takeOwnership(cmex::MxObject("radialcenter"));
			(*this)["COMmethod"].takeOwnership(cmex::MxObject("meanABS"));
			(*this)["DistanceFactor"].takeOwnership(mxCreateDoubleScalar(INFINITY));
			(*this)["LimFrac"].takeOwnership(mxCreateDoubleScalar(0.2));
			//(*this)["roiList"].
		}
	};

	class RoiTracker : public extras::async::ParamProcessor {
		
	};

}}