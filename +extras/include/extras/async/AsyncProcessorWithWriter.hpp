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
#include <extras/mxfile/mxfile_writer.hpp>
/*******************************************/

namespace extras {namespace async {

	class AsyncProcessorWithWriter : public AsyncProcessor {
	protected:
		/////////////////////////////
		// Save Results Related

		std::atomic_bool _SaveResults = false; //flag indicating results should be saved
		extras::mxfile::AsyncMxFileWriter _AsyncWriter; //instance of file writer

		/** Core method called by ProcessLoop() to handle tasks.
		* This function is responsible for calling ProcessNextTask()
		* it should return a bool specifying if there are more tasks to process
		* 
		* Unless you know what you are doing, it is not a good idea to redefine this method (even though it is virtual)
		* The method has been redefined here to include the ability to write the results to a file
		*/
		virtual bool ProcessLoopCore() {

			auto results = ProcessNextTask();

			// store result on results list
			std::lock_guard<std::mutex> rlock(ResultsListMutex);

			if (results.size()>0) {

				if (_SaveResults) { //write data if needed
					_AsyncWriter.writeArrays(results.size(), results);
				}

				ResultsList.push_front(std::move(results));

			}

			if (remainingTasks() < 1) {
				return false;
			}

			return true;
		}
	public:
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
		void resumeResultsWriter() { _AsyncWriter.resume(); }

		void saveResults(bool tf) { _SaveResults = tf; }
		bool saveResults() const { return _SaveResults; }
	};



	//! Implement mexInterface for AsyncProcessor
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of AsyncProcessorWithWriter*/
	class AsyncProcessorWithWriterInterface : public AsyncMexInterface<ObjType, ObjManager> {
		typedef AsyncMexInterface<ObjType, ObjManager> ParentType;
	protected:
		///////////////////////////////
		// File Writer

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
		AsyncMexInterface() {
			using namespace std::placeholders;

			///////////////////////////////
			// File Writer

			ParentType::addFunction("openResultsFile", std::bind(&AsyncProcessorWithWriterInterface::openResultsFile, this, _1, _2, _3, _4));
			ParentType::addFunction("isResultsFileOpen", std::bind(&AsyncProcessorWithWriterInterface::isResultsFileOpen, this, _1, _2, _3, _4));
			ParentType::addFunction("closeResultsFile", std::bind(&AsyncProcessorWithWriterInterface::closeResultsFile, this, _1, _2, _3, _4));
			ParentType::addFunction("clearUnsavedResults", std::bind(&AsyncProcessorWithWriterInterface::clearUnsavedResults, this, _1, _2, _3, _4));
			ParentType::addFunction("clearResultsWriterError", std::bind(&AsyncProcessorWithWriterInterface::clearResultsWriterError, this, _1, _2, _3, _4));
			ParentType::addFunction("getResultsWriterError", std::bind(&AsyncProcessorWithWriterInterface::getResultsWriterError, this, _1, _2, _3, _4));
			ParentType::addFunction("wasResultsWriterErrorThrown", std::bind(&AsyncProcessorWithWriterInterface::wasResultsWriterErrorThrown, this, _1, _2, _3, _4));
			ParentType::addFunction("isResultsWriterRunning", std::bind(&AsyncProcessorWithWriterInterface::isResultsWriterRunning, this, _1, _2, _3, _4));
			ParentType::addFunction("ResultsFilepath", std::bind(&AsyncProcessorWithWriterInterface::ResultsFilepath, this, _1, _2, _3, _4));
			ParentType::addFunction("pauseResultsWriter", std::bind(&AsyncProcessorWithWriterInterface::pauseResultsWriter, this, _1, _2, _3, _4));
			ParentType::addFunction("resumeResultsWriter", std::bind(&AsyncProcessorWithWriterInterface::resumeResultsWriter, this, _1, _2, _3, _4));
			ParentType::addFunction("saveResults", std::bind(&AsyncProcessorWithWriterInterface::saveResults, this, _1, _2, _3, _4));
		}
	};

}}