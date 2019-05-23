/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

/********************************************************************
COMPRESSION Includes
=====================================================================
This header depends on ZLIB for reading and writing files compressed
with the gz format. Therefore you need to have zlib built/installed
on your system.

A version of ZLIB is included with the ExtrasToolbox located in
.../+extras/external_libs/zlib
Look at that folder for build instructions.
Alternatively, is you are using a *nix-type system, you might have
better luck using your package manager to install zlib.

When building, be sure to include the location of zlib.h and to link to the
compiled zlib-lib files.
*********************************************/
#include "roiTracker.hpp"
/********************************************/

#include <splineroot/source/splineroot_mex.hpp>
#include <imradialavg/source/imradialavg_mex.hpp>

//#include <regex>
//#include <tinyexpr/tinyexpr.h> //used for expression evaluating in processing loop

namespace extras {namespace ParticleTracking {

	/** RoiTracker for 3D Tracking using splineroot LUT search
	 * In addition to the roiList fields used by RoiTracker (which are passed to RoiTracker to find XY locations),
	 * this class also checks if each LUT are defined for each roi. If they are, it computes the imradialavd around the
	 * determined XY coordinate and then finds the position in the LUT using splineroot.
	 *
	 * LUT results are added to each roi's LUT struct field
	 *
	 * The Structure of the resulting data will look like this:
	 *		result(n).
	 *				 .Window
	 *				 .UUID
	 *				 ... % all input fields, passed to output
	 *				 .X
	 *				 .Y
	 *				 .RadialAverage
	 *				 .LUT(k).
	 *						.MinR
	 *						.MaxR
	 *						.pp
	 *						.dpp
	 *						.... % all input fields passed to output
	 *						.Result.
	 *								.Z -> calculated position in lut
	 *								.varZ -> statistical variance of Z
	 *								.nItr -> number of iterations
	 *								.s -> last newton step size
	 *								.R2 -> last sq. residual
	 *								.dR2frac -> fractional change in sq residual at last step
	 *								.initR2 -> initial sq. residual from initial nearest knot guess
	*/
	class RoiTracker3D : public RoiTracker {
	protected:
		// Extend RoiTracker ProcessTask
		extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& TaskArgs, std::shared_ptr<const extras::cmex::ParameterMxMap> Params) {
			using namespace extras::cmex;

			// Find XY coordinates for particles
			extras::cmex::mxArrayGroup resultsGroup = extras::ParticleTracking::RoiTracker::ProcessTask(TaskArgs, Params);
			if (resultsGroup.size() == 0) { //handle no return values
				return resultsGroup;
			}

			MxStruct resultStruct(resultsGroup.getArray(0), true); //create alias to resultsGroup[0]

			if (!resultStruct.isfield("roiList")) { //no roiList, return
				return resultsGroup;
			}

			MxStruct roiList(resultStruct(0,"roiList").getmxarray(), true); // alias to "roiList" field in resultStruct (a.k.a. resultsGroup[0])

			if (!roiList.isfield("CentroidResult")) { //centroidresult not defined, skip
				return resultsGroup;
			}

			/*if (!roiList.isfield("X") || !roiList.isfield("Y")) { //X Y not defined, skip

			}*/

			// if no LUT then skip
			if (!roiList.isfield("LUT") ) {
				return resultsGroup;
			}

			////////////////
			// Get image
			const mxArray* mxI = nullptr;
			if (mxIsStruct(TaskArgs.getConstArray(0))) {
				mxI = mxGetField(TaskArgs.getConstArray(0), 0, "ImageData");
			}
			else {
				mxI = TaskArgs.getConstArray(0);
			}

			/////////////////////////////////////////////
			//loop over roi and calc z if needed
			for (size_t n = 0; n < roiList.numel(); n++) {
				////////////////////////////////////////////////////////
				// Prepare loop

				MxStruct CentroidResult(roiList(n, "CentroidResult").getmxarray()); //alias to CentroidResult in roiList(n)
				double x = CentroidResult(0, "X");
				double y = CentroidResult(0, "Y");

				if ( x==NAN || y==NAN ) { //did't find particle, skip
					continue;
				}

				// if LUT is empty, then skip
				if (roiList(n, "LUT").isempty()) {
					continue;
				}

				//make sure LUT is struct
				if (!roiList(n, "LUT").isstruct()) {
					throw(stacktrace_error(std::string("RoiTracker3D::ProcessTask(): ROI n=") + std::to_string(n) + "LUT Field is not a struct."));
				}

				MxStruct LUT_list(roiList(n, "LUT").getmxarray()); //Struct List of LUT

				if (!LUT_list.isfield("IsCalibrated")) {
					throw(extras::stacktrace_error(std::string("RoiTracker3D::ProcessTracking(): ROI n=") + std::to_string(n)
						+ "\nLUT Does not Contain IsCalibrated Field"
					));
				}

				////////////
				// Loop over all the LUT and determing the lowest MinR and largest MaxR
				int maxR = -1;
				int minR = INT_MAX;
				bool none_calibrated = true;
				for (size_t k = 0; k < LUT_list.numel(); k++) { //loop over LUT
					if ((bool)LUT_list(k, "IsCalibrated")) {
						none_calibrated = false;
						minR = std::min(minR, int(double(LUT_list(k, "MinR"))));
						maxR = std::max(maxR, int(double(LUT_list(k, "MaxR"))));
					}
				}

				if (none_calibrated) { //none of the lut are calibrated, continue to next roi
					continue;
				}

				/*if (!isfinite(minR)) {
					throw(extras::stacktrace_error(std::string("RoiTracker3D::ProcessTracking(): ROI n=") + std::to_string(n)
						+ std::string("\minR is not finite\n")
						+ std::string("minR: ") + std::to_string(minR)
					));
				}

				if (!isfinite(maxR)) {
					throw(extras::stacktrace_error(std::string("RoiTracker3D::ProcessTracking(): ROI n=") + std::to_string(n)
						+ std::string("\maxR is not finite\n")
						+ std::string("maxR: ") + std::to_string(maxR)
					));
				}*/

				// check minR & maxR are ok
				if (minR > maxR) {
					throw(extras::stacktrace_error(std::string("RoiTracker3D::ProcessTask(): ROI n=") + std::to_string(n) + std::string(" minR > maxR")));
				}

				//////////////
				// Computer radial avg
				auto radavg_result = radialavg(mxI, x-1, y-1, maxR, minR, 1);
				NumericArray<double>& imravg = std::get<0>(radavg_result);

				roiList(n, "RadialAverage") = imravg;
				roiList(n, "RadialAverage_rloc") = std::get<1>(radavg_result);

				////////////////////////////////////////////////////////////////////////////
				// Use splineroot to compute z
				//
				// Will add "Z" and other fields to LUT

				/////////////
				// Validate Fields & Parameters
				// check for pp field
				if (!LUT_list.isfield("pp")) {
					throw("RoiTracker3D::ProcessTask(): LUT struct does not contain 'pp' field");
				}

				//Default slineroot parameters
				double TOL = 0.001;
				if (Params->isparameter("splineroot_TOL")) {
					TOL = mxGetScalar(Params->operator[]("splineroot_TOL"));
				}
				double minStep = 20 * DBL_EPSILON;
				if (Params->isparameter("splineroot_minStep")) {
					minStep = mxGetScalar(Params->operator[]("splineroot_minStep"));
				}
				size_t maxItr = 10000;
				if (Params->isparameter("splineroot_maxItr")) {
					maxItr = mxGetScalar(Params->operator[]("splineroot_maxItr"));
				}
				double minR2frac = 0;// 0.00001;
				if (Params->isparameter("splineroot_minR2frac")) {
					minR2frac = mxGetScalar(Params->operator[]("splineroot_minR2frac"));
				}
				double MaxR2 = INFINITY;
				if (Params->isparameter("splineroot_MaxR2")) {
					minR2frac = mxGetScalar(Params->operator[]("splineroot_MaxR2"));
				}

				/////////////
				// Loop over LUT and compute
				for (size_t k = 0; k < LUT_list.numel(); k++) {
					if (!(bool)LUT_list(k, "IsCalibrated")) { //lut not calibrated, continue
						continue;
					}
					spline pp;
					if (createspline(&pp, LUT_list(k, "pp"))<0) {
						throw(extras::stacktrace_error(std::string("ROI:") + std::to_string(n)
							+ std::string(" LUT:") + std::to_string(k)
							+ std::string("\npp not a valid spline")));
					}

					spline dpp;
					bool free_dpp = false;
					char * dpp_calc = nullptr; //array noting which breaks of dpp have been calculated
					if (LUT_list.isfield("dpp")) {
						if (createspline(&dpp, LUT_list(k, "dpp"))<0) {
							throw(extras::stacktrace_error(std::string("ROI:") + std::to_string(n)
								+ std::string(" LUT:") + std::to_string(k)
								+ std::string("dpp not a valid spline")));
						}
						//set calc flag for all dpp coefs
						dpp_calc = (char*)malloc((dpp.nBreaks - 1) * sizeof(char));
						memset(dpp_calc, 1, dpp.nBreaks - 1);
					}
					else { //no dpp, need to create one
						dpp.dim = pp.dim;
						dpp.order = pp.order - 1;
						dpp.nBreaks = pp.nBreaks;
						dpp.breaks = pp.breaks;
						dpp.stride = (dpp.nBreaks - 1)*dpp.dim;
						dpp.coefs = (double*)malloc(dpp.stride*dpp.order * sizeof(double));
						free_dpp = true;

						//set calc flag for all dpp coefs to false
						dpp_calc = (char*)calloc((dpp.nBreaks - 1), sizeof(char));
					}

					/// Output variables
					double Z;
					double varZ;
					size_t nItr;
					double s;
					double R2;
					double dR2frac;
					double initR2;

					//////////
					// Normalize and find root
					if (LUT_list.isfield("NormalizeProfileData") && (bool)LUT_list(k, "NormalizeProfileData")) { //need to normalize

						size_t start_index = (double)LUT_list(k, "MinR") - minR; //starting index in imravg
						size_t dat_len = (double)LUT_list(k, "MaxR") - (double)LUT_list(k, "MinR") + 1;

						size_t norm_start = (double)LUT_list(k, "NormalizeProfileStartIndex") + start_index - 1;
						size_t norm_end = (double)LUT_list(k, "NormalizeProfileEndIndex") + start_index - 1;


						// compute avg over range
						double avg = 0;
						for (size_t jj = norm_start; jj <= norm_end; jj++) {
							avg += imravg.getdata()[jj];
						}
						avg /= (norm_end - norm_start + 1);
						std::vector<double> imr(dat_len);
						for (size_t jj = 0; jj < dat_len; jj++) {
							imr[jj] = imravg.getdata()[start_index + jj] / avg;
						}

						Z = splineroot(imr.data(),
							pp, dpp, dpp_calc,
							&varZ, pow(TOL, 2),
							maxItr, minStep, minR2frac, MaxR2,
							&nItr, &s, &R2, &dR2frac, &initR2);

					}
					else { //nothing needed, just process
						size_t start_index = (double)LUT_list(k, "MinR") - minR; //starting index in imravg
						const double* imr = &(imravg.getdata()[start_index]); //pointer to start of imravg data

						Z = splineroot(imr,
							pp, dpp, dpp_calc,
							&varZ, pow(TOL, 2),
							maxItr, minStep, minR2frac, MaxR2,
							&nItr, &s, &R2, &dR2frac, &initR2);

					}



					/////////////
					//set output fields
					MxStruct lutRes(1, { "Z","varZ","nItr","s","R2","dR2frac","initR2" });

					lutRes(0, "Z") = Z;
					lutRes(0, "varZ") = varZ;
					lutRes(0, "nItr") = nItr;
					lutRes(0, "s") = s;
					lutRes(0, "R2") = R2;
					lutRes(0, "dR2frac") = dR2frac;
					lutRes(0, "initR2") = initR2;

					LUT_list(k, "DepthResult") = lutRes.releaseArray();

					//cleanup dpp arrays
					free(dpp_calc);
					if (free_dpp) {
						free(dpp.coefs);
					}
				}
	
			}

			/////////////////
			// Return ResultsGroup
			return resultsGroup;
		}
	};
}}
