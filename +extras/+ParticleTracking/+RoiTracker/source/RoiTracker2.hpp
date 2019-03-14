/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "RoiParameterMap.hpp"

//////
// ParticleTracking Includes
// Be sure to add .../+extras/+ParticleTracking to your Include Path
#include <radialcenter/source/radialcenter_mex.hpp>
#include <barycenter/source/barycenter_mex.hpp>


namespace extras { namespace ParticleTracking {

	class RoiTracker : public extras::async::ParamProcessor {
	protected:
		//! Define ProcessTask method
		extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& TaskArgs, std::shared_ptr<const extras::cmex::ParameterMxMap> Params) {
			using namespace extras::cmex;

			// Cast Param to: shared_ptr<const RoiParameterMap>
			std::shared_ptr<const RoiParameterMap> ParamMap;
			if (!Params) { //Params were not initialized, just use an empty map
				ParamMap = std::make_shared<RoiParameterMap>();
			}
			else {
				ParamMap = std::dynamic_pointer_cast<const RoiParameterMap>(Params);
			}

			// Get Image
			const mxArray* img = nullptr;
			const mxArray* time = nullptr;
			if (mxIsStruct(TaskArgs.getArray(0))) {
				img = mxGetField(TaskArgs.getArray(0), 0, "ImageData");
				time = mxGetField(TaskArgs.getArray(0), 0, "Time");
			}
			else {
				img = TaskArgs.getArray(0);
			}

			MxStruct outStruct((*ParamMap)["roiList"]); //create copy of roiList for output;
			outStruct.makePersistent(); //make persisten now so that it doesn't get messed up by the fact that we are running in a thread

			/////////////////////////////
			// Process Image using appropriate method
			const char* xymethod = getstring((*ParamMap)["xyMethod"]).c_str();
			if (strcmpi("radialcenter", xymethod)==0) { //radial center
				
				rcdefs::RCparams rcP;
				rcP.XYc = ParamMap->get_XYc();
				rcP.RadiusFilter = ParamMap->get_RadiusFilter();
				rcP.COMmethod = ParticleTracking::string2COMmethod(getstring((*ParamMap)["COMmethod"]));
				rcP.DistanceFactor = mxGetScalar((*ParamMap)["DistanceFactor"]);

				auto rcOut =
					extras::ParticleTracking::radialcenter<extras::Array<double>>(
						img,
						*(ParamMap->get_WIND()),
						*(ParamMap->get_GP()),
						rcP);

				rcOut[0] += 1; //shift for 1-indexing
				rcOut[1] += 1; //shift for 1-indexing

							   //set field values
				for (size_t n = 0; n < ParamMap->get_WIND()->nRows(); ++n) {
					outStruct(n, "X") = rcOut[0][n];
					outStruct(n, "Y") = rcOut[1][n];

					NumericArray<double> vxy(2, 1);
					vxy(0) = rcOut[2](n, 0);
					vxy(1) = rcOut[2](n, 1);
					outStruct(n, "varXY") = vxy;

					outStruct(n, "RWR_N") = rcOut[3][n];
					outStruct(n, "xyMethod") = "radialcenter";
				}
			}
			else if (strcmpi("barycenter", xymethod) == 0) { //barrycenter

			}
			else {
				throw(std::runtime_error(std::string("invalid xyMethod: ") + std::string(xymethod)));
			}


			//////////////////
			// Return result
			mxArray* outmx = outStruct;
			return mxArrayGroup(1, &outmx);

		}
	public:

		//! default constructor changes pMap to point to an RoiParameterMap
		RoiTracker() {
			_pMap = std::dynamic_pointer_cast<extras::cmex::ParameterMxMap>(std::make_shared<RoiParameterMap>());
		}

		///////////////////////
		// Parameter Related

		//! add or replace persistent perameters
		//! changed from default.
		//! in this version parameter points to an RoiParameterMap
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {
			if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParamProcessor::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}
			std::shared_ptr<RoiParameterMap> newMap = std::make_shared<RoiParameterMap>(); // create new, empty parameter map;
			if (_pMap) { //_pMap is not nullptr
				newMap = std::make_shared<RoiParameterMap>(*std::dynamic_pointer_cast<RoiParameterMap>(_pMap)); // make a copy of the parametermap
			}

			newMap->setParameters(nrhs, prhs);

			_pMap = newMap;
		}

		// clear all parameters
		//! changed from default.
		//! in this version parameter points to an RoiParameterMap
		virtual void clearParameters() {
			_pMap = std::make_shared<RoiParameterMap>(); // create new, empty parameter map;
		}

		////////////////////////////////////////////////////////
		// Task Related

		//! push arguments to the task list.
		//! Each call to ProcessTask by the task thread will pop the "pushed" arguments
		//! off the stack and use them as arguments for ProcessTask(___)
		//!
		//! Modified here to assert that task either contains an image or contains a struct with "ImageData" and "Time" field
		virtual void pushTask(size_t nrhs, const mxArray* prhs[])
		{
			using namespace std;
			if (nrhs < 1) { return; }
			if (nrhs > 1) {
				throw(runtime_error("XYtracker::pushTask() only accepts one input. It should be an image, or a struct containing 'ImageData' and 'Time' fields"));
			}

			if (mxIsStruct(prhs[0])) {
				if (mxGetFieldNumber(prhs[0], "ImageData")<0) {
					throw(std::runtime_error("XYtracker::pushTask(), Image structuct must contain 'ImageData' field"));
				}
				if (mxGetFieldNumber(prhs[0], "Time")<0) {
					throw(std::runtime_error("XYtracker::pushTask(), Image structuct must contain 'Time' field"));
				}
			}

			// add task to the TaskList
			extras::async::ParamProcessor::pushTask(nrhs, prhs);
		}

	};

}}