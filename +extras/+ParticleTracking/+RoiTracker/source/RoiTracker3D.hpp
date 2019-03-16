/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "RoiTracker.hpp"
#include <splineroot/source/splineroot_mex.hpp>

namespace extras {namespace ParticleTracking {

	/** Extension of RoiParameterMap to include parameters for splineroot
	*/
	class RoiParameterMap3D : public RoiParameterMap {
	protected:

		double splineroot_TOL;
		double splineroot_minStep;
		double splineroot_maxItr;
		double splineroot_min_dR2frac;

		virtual void setFieldValue(const std::string& field, const mxArray* mxa) {

			if (false) {

			}
			else {
				RoiParameterMap::setFieldValue(field, mxa);
			}
		}
	public:

		//! Default constructor
		//! Adds ...
		RoiParameterMap3D() {
			(*this)["splineroot_TOL"].takeOwnership();
			(*this)["splineroot_minStep"].takeOwnership();
			(*this)["splineroot_maxItr"].takeOwnership();
			(*this)["splineroot_min_dR2frac"].takeOwnership();
		}
	};

	/** RoiTracker for 3D Tracking using splineroot LUT search
	*/
	class RoiTracker3D : public RoiTracker {

	};
}}
