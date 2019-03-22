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

#include <extras/mxfile/mxfile_writer.hpp>


namespace extras { namespace ParticleTracking {

	
	/** Asynchronous Processor for particle tracking in ROIs.
	 *	RoiTracker is intended to be used with the ParamProcessorInterface defined in ParamProcessor.hpp
	 *	Currently, the tracker expects to recieve persistent parameters (via the setParameter() method);
	 *	Parameters should include
	 *		'roiList',mex struct array containing "Window" field
	 *		'xyMethod','radialcenter' or 'barycenter' specifying which tracking routine to use
	 *		'COMmethod','meanabs','normal','gradmag' specifying what type of processing should be applied to the image before processing via radialcenter()
	 *		'DistanceFactor',val: distance factor used by radialcenter()
	 *		'LimFrac',val: Limit Fraction used by barycenter()
	 *
	 * Deriving from RoiTracker:
	 *	If you want to derive a class from RoiTracker (to add functionality)
	 *	you will probably want to start by overriding the ProcessTask(...) method.
	 *
	 *  Parameters are passed to the ProcessTask() method via shared_ptr to a 
	 *  specialized PersistentMxMap (RoiParameterMap).
	 *	RoiParameterMaps allow the end user to pass any name,value argument pairs (like the standard PersistentMxMap)
	 *  however, it intercepts the specialized parameters.
	 *  Therefore if you want to add some ability to ProcessTask() that will need other parameters, you don't necessarily need
	 *  to also define a custom PersistentMxMap.
	*/
	class RoiTracker : public extras::async::ParamProcessor {
	protected:

		std::atomic_bool _IncludeImageInResults = false;

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
			if (mxIsStruct(TaskArgs.getConstArray(0))) {
				img = mxGetField(TaskArgs.getConstArray(0), 0, "ImageData");
				time = mxGetField(TaskArgs.getConstArray(0), 0, "Time");
			}
			else {
				img = TaskArgs.getConstArray(0);
			}

			MxStruct outStruct(ParamMap->get_roiList()); //create copy of roiList for output;
			outStruct.managearray(); //make outStruct editable
			outStruct.makePersistent();//make persistent now so that it doesn't get messed up by the fact that we are running in a thread

			/////////////////////////////
			// Process Image using appropriate method

			switch (ParamMap->get_xyMethod()) {
			case XY_FUNCTION::RADIALCENTER:
			{
				rcdefs::RCparams rcP;
				rcP.XYc = ParamMap->get_XYc();
				rcP.RadiusFilter = ParamMap->get_RadiusFilter();
				rcP.COMmethod = ParamMap->get_COMmethod();
				rcP.DistanceFactor = ParamMap->get_DistanceFactor();

				auto rcOut = radialcenter<extras::Array<double>>(
									img,
									*(ParamMap->get_WIND()),
									*(ParamMap->get_GP()),
									rcP
							);

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
				break;
			case XY_FUNCTION::BARYCENTER:
				break;
			default:
				throw("Undefined xyMethod.");
			}

			//////////////////
			// Return result

			mxArrayGroup results(1);
			results.ownArray(0,outStruct);

			if (_IncludeImageInResults) {
				results.push_back(TaskArgs.getConstArray(0));
			}

			if (_SaveResults) {
				_AsyncWriter.writeArrays(results.size(), results);
			}

			return results;

		}

		std::atomic_bool _SaveResults = false;
		extras::mxfile::AsyncMxFileWriter _AsyncWriter;
	public:

		~RoiTracker() {
			if (!_AsyncWriter.isFileOpen()) {
				_AsyncWriter.cancelRemainingTasks();
			}
		}

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

		//! check if image should be included with results (true=yes, false=no)
		bool includeImageData() const { return _IncludeImageInResults; }

		//! set if image should be included with results (true=yes, false=no)
		bool includeImageData(bool includedImage) {
			_IncludeImageInResults = includedImage;
			return _IncludeImageInResults;
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

		///////////////////////////////
		// File Writer

		void openResultsFile(std::string filepath) {
			_AsyncWriter.openFile(filepath);
		}

		bool isResultsFileOpen() const {
			return _AsyncWriter.isFileOpen();
		}

		void closeResultsFile() {
			_AsyncWriter.closeFile();
		}

		void clearUnsavedResults() {
			_AsyncWriter.cancelRemainingTasks();
		}

		void clearResultsWriterError() {
			_AsyncWriter.clearError();
		}

		std::exception_ptr getResultsWriterError() {
			return _AsyncWriter.getError();
		}

		bool wasResultsWriterErrorThrown() {
			return _AsyncWriter.wasErrorThrown();
		}

		bool isResultsWriterRunning() {
			return _AsyncWriter.running();
		}

		std::string ResultsFilepath() const { return _AsyncWriter.filepath(); }

		void pauseResultsWriter() { _AsyncWriter.pause(); }
		void resumeResultsWriter() { _AsyncWriter.resume();}

		void saveResults(bool tf) {_SaveResults = tf;}
		bool saveResults() const { return _SaveResults; }
	};

	/** Extend the ParamProcessorInterface for use with RoiTracker type objects
	 * Adds IncludeImageData method to the mexInterface
	 * 
	 * Usage: template ObjManager should be a reference to a manager for RoiTracker-class.
	*/
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager>
	class RoiTrackerInterface :public extras::async::ParamProcessorInterface<ObjType, ObjManager> {
		typedef extras::async::ParamProcessorInterface<ObjType, ObjManager> ParentType;
	protected:
		void IncludeImageData(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			auto objPtr = ParentType::getObjectPtr(nrhs, prhs);
			if (nrhs > 1) { //setting value
				if (!mxIsScalar(prhs[1])) {
					throw("Cannot set IncludeImageData. Argument must be scalar and convertable to logical.");
				}
				bool res = objPtr->includeImageData(mxGetScalar(prhs[1]));
				plhs[0] = mxCreateLogicalScalar(res);
			}
			else {
				bool res = objPtr->includeImageData();
				plhs[0] = mxCreateLogicalScalar(res);
			}
		}
		void openResultsFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			if (nrhs < 2) {
				throw("AsyncMxFileReaderInterface::openFile() required 2 inputs");
			}
			ParentType::getObjectPtr(nrhs, prhs)->openResultsFile(cmex::getstring(prhs[1]));
		}
		void isResultsFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool isopen = ParentType::getObjectPtr(nrhs, prhs)->isResultsFileOpen();
			plhs[0] = mxCreateLogicalScalar(isopen);
		}
		void closeResultsFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->closeResultsFile();
		}
		void clearUnsavedResults(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearUnsavedResults();
		}
		void clearResultsWriterError(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearResultsWriterError();
		}
		void getResultsWriterError(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			std::exception_ptr err = ParentType::getObjectPtr(nrhs, prhs)->getResultsWriterError();

			if (err == nullptr) { //no errors, return empty
				plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
				return;
			}

			//convert exception ptr to struct
			try {
				rethrow_exception(err);
			}
			catch (const std::exception& e) {
				const char* fields[] = { "identifier","message" };
				mxArray* out = mxCreateStructMatrix(1, 1, 2, fields);
				mxSetField(out, 0, "identifier", mxCreateString("ProcessingError"));
				mxSetField(out, 0, "message", mxCreateString(e.what()));

				plhs[0] = out;
			}
		}
		void wasResultsWriterErrorThrown(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->wasResultsWriterErrorThrown());
		}
		void isResultsWriterRunning(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->isResultsWriterRunning());
		}
		void ResultsFilepath(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			cmex::MxObject fpth = ParentType::getObjectPtr(nrhs, prhs)->ResultsFilepath();
			plhs[0] = fpth;
		}
		void pauseResultsWriter(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->pauseResultsWriter();
		}
		void resumeResultsWriter(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->resumeResultsWriter();
		}
		void saveResults(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			if (nrhs < 2) {
				plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->saveResults());
			}
			else {
				ParentType::getObjectPtr(nrhs, prhs)->saveResults(mxGetScalar(prhs[1]));
			}
		}
	public:
		RoiTrackerInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("IncludeImageData", std::bind(&RoiTrackerInterface::IncludeImageData, this, _1, _2, _3, _4));
			ParentType::addFunction("openResultsFile", std::bind(&RoiTrackerInterface::openResultsFile, this, _1, _2, _3, _4));
			ParentType::addFunction("isResultsFileOpen", std::bind(&RoiTrackerInterface::isResultsFileOpen, this, _1, _2, _3, _4));
			ParentType::addFunction("closeResultsFile", std::bind(&RoiTrackerInterface::closeResultsFile, this, _1, _2, _3, _4));
			ParentType::addFunction("clearUnsavedResults", std::bind(&RoiTrackerInterface::clearUnsavedResults, this, _1, _2, _3, _4));
			ParentType::addFunction("clearResultsWriterError", std::bind(&RoiTrackerInterface::clearResultsWriterError, this, _1, _2, _3, _4));
			ParentType::addFunction("getResultsWriterError", std::bind(&RoiTrackerInterface::getResultsWriterError, this, _1, _2, _3, _4));
			ParentType::addFunction("wasResultsWriterErrorThrown", std::bind(&RoiTrackerInterface::wasResultsWriterErrorThrown, this, _1, _2, _3, _4));
			ParentType::addFunction("isResultsWriterRunning", std::bind(&RoiTrackerInterface::isResultsWriterRunning, this, _1, _2, _3, _4));
			ParentType::addFunction("ResultsFilepath", std::bind(&RoiTrackerInterface::ResultsFilepath, this, _1, _2, _3, _4));
			ParentType::addFunction("pauseResultsWriter", std::bind(&RoiTrackerInterface::pauseResultsWriter, this, _1, _2, _3, _4));
			ParentType::addFunction("resumeResultsWriter", std::bind(&RoiTrackerInterface::resumeResultsWriter, this, _1, _2, _3, _4));
			ParentType::addFunction("saveResults", std::bind(&RoiTrackerInterface::saveResults, this, _1, _2, _3, _4));
		}
	};

}}