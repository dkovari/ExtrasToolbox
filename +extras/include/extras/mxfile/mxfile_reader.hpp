/*----------------------------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari & James Merrill, Emory University
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include <cstdio>
#include <mex.h>
#include <list>
#include <string>
#include <vector>

#include <extras/cmex/mxobject.hpp>
#include <extras/cmex/mxClassIDhelpers.hpp>
#include <extras/numeric.hpp>
#include <extras/SessionManager/mexInterface.hpp>
#include <extras/cmex/mxArrayGroup.hpp>
#include <extras/cmex/MxCellArray.hpp>

/********************************************************************
COMPRESSION Includes & Libs
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
#include <zlib.h>
/********************************************/

namespace extras { namespace mxfile {
		
	//! Wrapper for handling generic file reader
	//! can be extended to support other file reading functions (e.g. zlib)
	class FILE_ReadPointer {
	protected:
		FILE* _fp = nullptr;
	public:
		FILE_ReadPointer(FILE* fp) :_fp(fp) {};

		//! attempts to read data from fp into dst
		//! returns number of bytes read
		virtual size_t read(void* dst, size_t nbytes) {
			return std::fread(dst, 1, nbytes, _fp);
		}

		//! return true if eof has been reached
		virtual bool eof() {
			return std::feof(_fp);
		}

		virtual const char* error_msg() {
			return std::strerror(std::ferror(_fp));
		}

		//! return FILE*
		FILE* getFP() const {
			return _fp;
		}
	};

	//! Wrapper for handling generic file reader
	//! this class extends FILE_ReadPointer to use zlib's gzread function
	class GZFILE_ReadPointer:public FILE_ReadPointer {
		friend class MxFileReader;
	protected:
		gzFile _fp = NULL;

		void clearFP() {
			_fp = NULL;
		}
	public:
		GZFILE_ReadPointer(gzFile fp):FILE_ReadPointer(nullptr),_fp(fp) {};

		//! attempts to read data from fp into dst
		//! returns number of bytes read
		virtual size_t read(void* dst, size_t nbytes) {
			return gzread(_fp, dst, nbytes);
		}

		//! return true if eof has been reached
		virtual bool eof() {
			return gzeof(_fp);
		}

		virtual const char* error_msg() {
			int eno;
			return gzerror(_fp, &eno);
		}

		//! return gzFile (hides inherited getFP()
		gzFile getFP() const {
			return _fp;
		}

		//! return byte offset in the file;
		size_t getOffset() const {
			return gzoffset(_fp);
		}
	};

	//! Read next mxArray from the file
	//! FP: reference to file pointer wrapper
	//! (optional) calledRecursively=false: specify if readNext is beinging called by recursion
	//!		when called recursively the function will throw an exception if EOF is reached when trying to read
	//!		the first byte of the mxarray header
	//!
	//! Otherwise, (default) the function simply returns and empty mxArray
	extras::cmex::MxObject readNext(FILE_ReadPointer& FP,bool calledRecursively=false) {
		extras::cmex::MxObject out;

		// read type
		mxClassID thisClassID;
		uint8_t tmp;
		if (FP.read(&tmp, 1) < 1) {
			if (FP.eof()) {
				if (calledRecursively) {
					throw(extras::stacktrace_error("readNext(): at EOF and called recursively"));
				}
				return (mxArray*)nullptr;//return empty array;
			}
			else {
				throw(extras::stacktrace_error(FP.error_msg()));
			}
		}
		thisClassID = (mxClassID)tmp;

		//read ndim
		size_t ndim;
		if (FP.read(&ndim, sizeof(size_t)) < sizeof(size_t)) {
			if (FP.eof()) {
				throw(extras::stacktrace_error("reached end of file while reading ndim"));
			}
			else {
				throw(extras::stacktrace_error(FP.error_msg()));
			}
		}

		//read dims
		std::vector<size_t> dims(ndim);
		if (FP.read(dims.data(), sizeof(size_t)*ndim) < sizeof(size_t)*ndim) {
			if (FP.eof()) {
				throw(extras::stacktrace_error("reached end of file while reading dims"));
			}
			else {
				throw(extras::stacktrace_error(FP.error_msg()));
			}
		}

		//create array to hold data
		switch (thisClassID) {
			case mxCELL_CLASS:
			{
				out.own(mxCreateCellArray(ndim, dims.data()));
				for (size_t n = 0; n < out.numel(); n++) {
					mxSetCell(out.getmxarray(), n, readNext(FP,true));
				}
			}
				break;
			case mxSTRUCT_CLASS:
			{
				//get nfields
				size_t nfields;
				if (FP.read(&nfields, sizeof(size_t)) < sizeof(size_t)) {
					if (FP.eof()) {
						throw(extras::stacktrace_error("reached end of file while reading nfields"));
					}
					else {
						throw(extras::stacktrace_error(FP.error_msg()));
					}
				}

				std::vector<std::vector<char>> fieldnames(nfields); //vector of strings in which names are stored
				std::vector<const char*> fnames(nfields); //vector of const char* used to call mxCreateStruct...
				for (size_t f = 0; f < nfields; f++) { //loop and read names
					size_t len; //length of fieldname including null terminator
					if (FP.read(&len, sizeof(size_t)) < sizeof(size_t)) {
						if (FP.eof()) {
							throw(extras::stacktrace_error("reached end of file while reading length of fieldname"));
						}
						else {
							throw(extras::stacktrace_error(FP.error_msg()));
						}
					}

					fieldnames[f].resize(len); //read fieldname, including null terminator
					if (FP.read(fieldnames[f].data(), len) < len) {
						if (FP.eof()) {
							throw(extras::stacktrace_error("reached end of file while reading lfieldname"));
						}
						else {
							throw(extras::stacktrace_error(FP.error_msg()));
						}
					}
					fnames[f] = fieldnames[f].data(); //set pointer array
				}

				out.own(mxCreateStructArray(ndim, dims.data(), nfields, fnames.data()));

				/// Loop and read fields
				for (size_t j = 0; j < out.numel(); ++j) {
					for (size_t k = 0; k < nfields; ++k) {
						mxSetFieldByNumber(out.getmxarray(), j, k, readNext(FP,true));
					}
				}

			}
				break;
			default:
			{
				//read complexity
				uint8_t isComplex;
				if (FP.read(&isComplex, 1) < 1) {
					if (FP.eof()) {
						throw(extras::stacktrace_error("reached end of file while reading isComplex"));
					}
					else {
						throw(extras::stacktrace_error(FP.error_msg()));
					}
				}

				//read interleaved
				uint8_t interFlag;
				if (FP.read(&interFlag, 1) < 1) {
					if (FP.eof()) {
						throw(extras::stacktrace_error("reached end of file while reading interFlag"));
					}
					else {
						throw(extras::stacktrace_error(FP.error_msg()));
					}
				}

				if (thisClassID == mxCHAR_CLASS) { //handle char
					out.own(mxCreateCharArray(ndim, dims.data()));
					size_t elsz = out.elementBytes();
					if (FP.read(mxGetData(out.getmxarray()), elsz*out.numel()) < elsz*out.numel()) {
						if (FP.eof()) {
							throw(extras::stacktrace_error("reached end of file while reading char data"));
						}
						else {
							throw(extras::stacktrace_error(FP.error_msg()));
						}
					}
				}
				else if (!isComplex) { //real
					out.own(mxCreateNumericArray(ndim, dims.data(), thisClassID, mxREAL));
					size_t elsz = out.elementBytes();
					if (FP.read(mxGetData(out.getmxarray()), elsz*out.numel()) < elsz*out.numel()) {
						if (FP.eof()) {
							throw(extras::stacktrace_error("reached end of file while reading real numeric data"));
						}
						else {
							throw(extras::stacktrace_error(FP.error_msg()));
						}
					}
				}
				else { //complex
					size_t elsz_real = cmex::elementBytes(thisClassID, false);
					size_t numel = prod(dims);
					if (interFlag) { //data is interleaved
						char* data = (char*)mxMalloc(numel*elsz_real * 2);
						if (FP.read(data, numel*elsz_real * 2) < numel*elsz_real * 2) {
							if (FP.eof()) {
								mxFree(data); //cleanup out memory mess
								throw(extras::stacktrace_error("reached end of file while reading interleaved complex numeric data"));
							}
							else {
								mxFree(data); //cleanup out memory mess
								throw(extras::stacktrace_error(FP.error_msg()));
							}
						}
						// create array and set data
						mxArray* tmp = mxCreateNumericMatrix(0, 0, thisClassID, mxCOMPLEX);
						mxSetDimensions(tmp, dims.data(), ndim);
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved, just do a regular set
						mxSetData(tmp, data);
#else //data was interleaved, but MATLAB is not
						char* realData = (char*)mxMalloc(numel*elsz_real);
						char* imagData = (char*)mxMalloc(numel*elsz_real);

						//copy elements
						for (size_t n = 0; n < numel; n++) {
							std::memcpy(&(realData[n*elsz_real]), &(data[(n + 0)*elsz_real]), elsz_real); //copy real element
							std::memcpy(&(imagData[n*elsz_real]), &(data[(n + 1)*elsz_real]), elsz_real); //copy imag element
						}
						mxSetData(tmp, realData); //set real data
						mxSetImagData(tmp, imagData); //set imag data
						mxFree(data); //free data holding interleaved, we arent using it anymore
#endif
						out.own(tmp);
					}
					else { //data is not interleaved
						char* realData = (char*)mxMalloc(numel*elsz_real);
						char* imagData = (char*)mxMalloc(numel*elsz_real);
						if (FP.read(realData, numel*elsz_real) < numel*elsz_real) { //read real
							if (FP.eof()) {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw(extras::stacktrace_error("reached end of file while reading non-interleaved real data for complex numeric"));
							}
							else {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw(extras::stacktrace_error(FP.error_msg()));
							}
						}
						if (FP.read(imagData, numel*elsz_real) < numel*elsz_real) { //read imag
							if (FP.eof()) {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw(extras::stacktrace_error("reached end of file while reading non-interleaved imag data for complex numeric"));
							}
							else {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw(extras::stacktrace_error(FP.error_msg()));
							}
						}

						mxArray* tmp = mxCreateNumericMatrix(0, 0, thisClassID, mxCOMPLEX);
						mxSetDimensions(tmp, dims.data(), ndim);

#if MX_HAS_INTERLEAVED_COMPLEX //data was not interleaved, but MATLAB is
						char* interData = (char*)mxMalloc(numel*elsz_real * 2);
						//copy elements
						for (size_t n = 0; n < numel; n++) {
								std::memcpy(&(interData[(n + 0)*elsz_real]),&(realData[n*elsz_real]),elsz_real); //copy real element
								std::memcpy(&(interData[(n + 1)*elsz_real]),&(imagData[n*elsz_real]), elsz_real); //copy imag element
						}

						mxSetData(tmp, interData);
						mxFree(realData);
						mxFree(imagData);
#else //both are not interleaved
						mxSetData(tmp, realData);
						mxSetImagData(tmp, imagData);
#endif
						out.own(tmp);

					}
				}
			}
		}

		return out;
	}


	class MxFileReader {
	protected:
		std::mutex _RPmutex; //mutex protecting _ReadPointer
		mutable GZFILE_ReadPointer _ReadPointer; //pointer class for gzFile, proteced by locks using _RPmutex
		std::string _filepath;
		size_t _compressedSize;
		size_t _currentCompressedPosition;
	public:
		//! return true if file is open
		bool isFileOpen() const {
			return _ReadPointer.getFP() != NULL;
		}

		//! returns copy of filepath string
		std::string filepath() const {
			return _filepath;
		}

		bool isEOF() const {
			return _ReadPointer.eof();
		}

		////////////////
		// Constructor

		//! default constructor
		MxFileReader() :_ReadPointer(NULL) {};

		/////////////////
		//! Destructor
		//! closes file
		virtual ~MxFileReader() {
			closeFile();
		}

		//! close the file
		void closeFile() {
			if (isFileOpen()) {
				std::lock_guard<std::mutex> lock(_RPmutex);
				gzclose(_ReadPointer.getFP());
				_ReadPointer.clearFP();
			}
		}

		//! open specified file for reading
		void openFile(std::string fpth) {
			closeFile();

			//determine size of file
			FILE* fp = fopen(fpth.c_str(), "rb");
			if (fp == nullptr) {
				throw(extras::stacktrace_error(std::string("MxFileReader::openFile(): returned null, file:'") + std::string(fpth) + std::string("' could not be opened.")));
			}
			if (fseek(fp, 0, SEEK_END) != 0) {
				fclose(fp);
				throw(extras::stacktrace_error(std::string("MxFileReader::openFile(): file:'") + std::string(fpth) + std::string("' Could not determine file size.")));
			}
			_compressedSize = ftell(fp);
			fclose(fp); //close


			gzFile gzf = gzopen(fpth.c_str(), "rb");
			if (gzf == NULL) {
				throw(extras::stacktrace_error(std::string("MxFileReader::openFile(): returned null, file:'") + std::string(fpth) + std::string("' could not be opened.")));
			}
			std::lock_guard<std::mutex> lock(_RPmutex);
			_ReadPointer = GZFILE_ReadPointer(gzf);
			_filepath = fpth;
			_currentCompressedPosition = _ReadPointer.getOffset();
		}

		//!open file for writing (using MATLAB args)
		void openFile(size_t nrhs, const mxArray** prhs) {
			if (nrhs < 1) {
				throw(extras::stacktrace_error("MxFileReader::openWriter() expected one argument"));
			}
			openFile(extras::cmex::getstring(prhs[0]));
		}

		//! attempt to read next array from file
		extras::cmex::MxObject readNextArray() {
			if (!isFileOpen()) {
				throw(extras::stacktrace_error(std::string("MxFileReader::readNextArray() file:'") + _filepath + std::string("' is not open.")));
			}
			if (isEOF()) {
				throw(extras::stacktrace_error(std::string("MxFileReader::readNextArray() File:'") + _filepath + std::string("'. Reached End of File.")));
			}
			cmex::MxObject out = readNext(_ReadPointer);
			_currentCompressedPosition = _ReadPointer.getOffset();
			return out;
		}

		//! get compressed size of file
		size_t getCompressedSize() const {
			if (!isFileOpen()) {
				throw(extras::stacktrace_error(std::string("MxFileReader::readNextArray() file:'") + _filepath + std::string("' is not open.")));
			}
			return _compressedSize;
		}

		//! get current (compressed) position in file
		size_t getPositionInFile() const {
			if (!isFileOpen()) {
				throw(extras::stacktrace_error(std::string("MxFileReader::readNextArray() file:'") + _filepath + std::string("' is not open.")));
			}
			return _currentCompressedPosition;
		}

		//! fraction of load progress (via ratio remaining compressed data)
		double loadProgress() const {
			if (!isFileOpen()) {
				return 0;
			}
			return (double)_currentCompressedPosition / (double)_compressedSize;
			
		}
	};

	/////////////////////////

	//! implement mexInterface for MxFileReader
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of MxFileReader*/
	class MxFileReaderInterface : public SessionManager::mexInterface<ObjType, ObjManager> {
		typedef SessionManager::mexInterface<ObjType, ObjManager> ParentType;
	protected:
		void openFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->openFile(nrhs - 1, &prhs[1]);
		}
		void closeFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->closeFile();
		}
		void filepath(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			cmex::MxObject fpth = ParentType::getObjectPtr(nrhs, prhs)->filepath();
			plhs[0] = fpth;
		}
		void isEOF(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool iseof = ParentType::getObjectPtr(nrhs, prhs)->isEOF();
			plhs[0] = mxCreateLogicalScalar(iseof);
		}
		void readNextArray(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			cmex::MxObject out = ParentType::getObjectPtr(nrhs, prhs)->readNextArray();
			if (out.getmxarray() == nullptr) {
				if (ParentType::getObjectPtr(nrhs, prhs)->isEOF()) {
					plhs[0] = mxCreateCellMatrix(0, 0);
					return;
				}
				throw(extras::stacktrace_error(std::string("MxFileReader::readNextArray() File:'") + ParentType::getObjectPtr(nrhs, prhs)->filepath() + std::string("'. Recieved Nullptr and not at EOF.")));
			}
			plhs[0] = out;
		}
		void isFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool isopen = ParentType::getObjectPtr(nrhs, prhs)->isFileOpen();
			plhs[0] = mxCreateLogicalScalar(isopen);
		}
		void getCompressedSize(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			size_t val = ParentType::getObjectPtr(nrhs, prhs)->getCompressedSize();
			plhs[0] = mxCreateDoubleScalar(val);
		}
		void getPositionInFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			size_t val = ParentType::getObjectPtr(nrhs, prhs)->getPositionInFile();
			plhs[0] = mxCreateDoubleScalar(val);
		}
		void loadProgress(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			size_t val = ParentType::getObjectPtr(nrhs, prhs)->loadProgress();
			plhs[0] = mxCreateDoubleScalar(val);
		}

	public:
		MxFileReaderInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("openFile", std::bind(&MxFileReaderInterface::openFile, this, _1, _2, _3, _4));
			ParentType::addFunction("closeFile", std::bind(&MxFileReaderInterface::closeFile, this, _1, _2, _3, _4));
			ParentType::addFunction("filepath", std::bind(&MxFileReaderInterface::filepath, this, _1, _2, _3, _4));
			ParentType::addFunction("readNextArray", std::bind(&MxFileReaderInterface::readNextArray, this, _1, _2, _3, _4));
			ParentType::addFunction("isEOF", std::bind(&MxFileReaderInterface::isEOF, this, _1, _2, _3, _4));
			ParentType::addFunction("isFileOpen", std::bind(&MxFileReaderInterface::isFileOpen, this, _1, _2, _3, _4));
			ParentType::addFunction("getCompressedSize", std::bind(&MxFileReaderInterface::getCompressedSize, this, _1, _2, _3, _4));
			ParentType::addFunction("getPositionInFile", std::bind(&MxFileReaderInterface::getPositionInFile, this, _1, _2, _3, _4));
			ParentType::addFunction("loadProgress", std::bind(&MxFileReaderInterface::loadProgress, this, _1, _2, _3, _4));
		}
	};


	////////////////////////////////////////////
	// Async Reader

	//! Asynchronous File Reader
	class AsyncMxFileReader {
	protected:
		mutable std::mutex _ResultsGroupMutex; //mutex lock for _ResultsGroup
		cmex::mxArrayGroup _ResultsGroup; //protected by _ResultsGroupMutex
#ifdef _DEBUG
		std::vector<const mxArray*> _ResPtrs;
		std::vector<const mxArray*> _ResultsPtrs;
#endif

		MxFileReader _Reader; //read for loading data from file

		std::thread mThread;
		std::atomic_bool ProcessRunning = false; //flag for stopping to processing loop
		std::atomic_bool _atEOF = false;

		std::atomic_bool ErrorFlag;
		std::mutex LastErrorMutex;
		std::exception_ptr LastError = nullptr;

		void LoadDataLoop() {
			try {
				while (_Reader.isFileOpen() && ProcessRunning) {
					if (_Reader.isEOF()) {
						_atEOF = true;
						ProcessRunning = false;
						return;
					}
					cmex::MxObject res = _Reader.readNextArray(); //read next data
					if (res.getmxarray() == nullptr) {
						if (_Reader.isEOF()) {
							_atEOF = true;
							ProcessRunning = false;
							return;
						}
						throw(extras::stacktrace_error("LoadDataLoop(): Recieved MxObject with _mxptr==nullptr and not at EOF"));
					}
#ifdef _DEBUG
					_ResPtrs.push_back(res.getmxarray());
#endif
					std::lock_guard<std::mutex> rlock(_ResultsGroupMutex); //lock results
					_ResultsGroup.move_back(res.releaseArray()); //move data to back of results list

#ifdef _DEBUG
					_ResultsPtrs.push_back(_ResultsGroup.getBack());
#endif
				}
			}
			catch (...) {
				ProcessRunning = false;
				if (_Reader.isEOF()) { //just at end of file, no more data to read
					_atEOF = true;
					ProcessRunning = false;
					return;
				}
				ErrorFlag = true;
				std::lock_guard<std::mutex> lock(LastErrorMutex);
				LastError = std::current_exception();
				return;
			}
			ProcessRunning = false;
		}

		//! immediately halts LoadDataLoop
		virtual void StopProcessor() {
			ProcessRunning = false;
			std::this_thread::sleep_for(std::chrono::microseconds(1000)); //let some time pass
			if (mThread.joinable()) {
				mexPrintf("\tHalting AsyncMxFileReader");
				mexEvalString("pause(0.2)");
				mThread.join();
				mexPrintf("...done.\n");
				mexEvalString("pause(0.1)");
			}
		}

		//! (re)launch thread
		virtual void StartProcessor() {
			StopProcessor();
			if (!_Reader.isFileOpen()) {
				throw(extras::stacktrace_error("File is not open, cannot StartProcessor()"));
			}
			ErrorFlag = false;
			ProcessRunning = true;
			try {
				mThread = std::thread(&AsyncMxFileReader::LoadDataLoop, this);
			}
			catch (...) {
				ProcessRunning = false;
				throw;
			}
		}

		//! clear loaded data
		void clearLoadedData() {
			std::lock_guard<std::mutex> rlock(_ResultsGroupMutex); //lock results
			_ResultsGroup.clear();
		}

	public:
		//! destructor
		virtual ~AsyncMxFileReader() {
			StopProcessor();
			_Reader.closeFile();
		}

		//! get compressed size of file
		size_t getCompressedSize() const {return _Reader.getCompressedSize();}

		//! get current (compressed) position in file
		size_t getPositionInFile() const {return _Reader.getPositionInFile();}

		//! fraction of load progress (via ratio remaining compressed data)
		double loadProgress() const {return _Reader.loadProgress();}

		//! filepath used by reader
		std::string filepath() const {
			return _Reader.filepath();
		}

		//check if reached end of file
		bool isEOF() const { return _atEOF; }

		bool isFileOpen() const{ return _Reader.isFileOpen(); }

		//! check if thread is still running
		bool threadRunning() const { return ProcessRunning; }

		//! open specified file for reading
		//! clears any loaded arrays waiting in the Results buffer
		void openFile(std::string fpth) {
			StopProcessor();
			clearLoadedData();
			_Reader.openFile(fpth);
			_atEOF = _Reader.isEOF();
		}

		//! open specified file for reading
		void closeFile() {
			StopProcessor();
			_Reader.closeFile();
		}

		//!start or continue loading data
		void loadData() {
			if (!ProcessRunning) {
				StartProcessor();
			}
		}
		//!start or continue loading data
		void resume() { loadData(); }

		//!pause data loading
		void pause() {
			StopProcessor();
		}

		//! stop loading and reset position back to begining
		//! also frees any data remaining in the result array
		void cancel() {
			if (!_Reader.isFileOpen()) {
				return;
			}
			StopProcessor();
			auto fpth = filepath();
			closeFile();
			openFile(fpth);
		}

		//! returns number of arrays loaded and awaiting copy to MATLAB
		size_t numberOfArraysLoaded() const {
			return _ResultsGroup.size();
		}

		//! gets loaded data but does not clear the mxArrayGroup holding results
		//!if a single array is loaded it is returned
		//! otherwise a cell containing all the loaded arrays are returned;
		cmex::MxObject getLoadedData() const {
			std::lock_guard<std::mutex> rlock(_ResultsGroupMutex); //lock results (pauses loading loop)
			if (_ResultsGroup.size() < 0) {
				return cmex::MxCellArray(); //return empty cell;
				//throw("AsyncMxFileReader::getLoadedData(): No Arrays to copy");
			}

			if (_ResultsGroup.size() == 1) { //only one array, don't return cell
				return _ResultsGroup[0];
			}

			cmex::MxCellArray out;// = cmex::MxCellArray({ _ResultsGroup.size(),1 });
			out.own(mxCreateCellMatrix(_ResultsGroup.size(), 1));
			//copy results to cell array
			for (size_t n = 0; n < _ResultsGroup.size(); n++) {
				mxSetCell(out.getmxarray(), n, mxDuplicateArray(_ResultsGroup[n]));
			}
			return out;
		}

		//! gets loaded data and clears the mxArrayGroup
		//!if a single array is loaded it is returned
		//! otherwise a cell containing all the loaded arrays are returned;
		cmex::MxObject getLoadedDataAndClear() {
			//cmex::MxObject out = getLoadedData();
			//::lock_guard<std::mutex> rlock(_ResultsGroupMutex); //lock results (pauses loading loop)
			std::lock_guard<std::mutex> rlock(_ResultsGroupMutex); //lock results (pauses loading loop)
			if (_ResultsGroup.size() < 0) {
				return cmex::MxCellArray(); //return empty cell;
				//throw("AsyncMxFileReader::getLoadedData(): No Arrays to copy");
			}

			if (_ResultsGroup.size() == 1) { //only one array, don't return cell
				return _ResultsGroup[0];
			}

			cmex::MxObject out;// = cmex::MxCellArray({ _ResultsGroup.size(),1 });
			out.own(mxCreateCellMatrix(_ResultsGroup.size(), 1));
			//copy results to cell array
			for (size_t n = 0; n < _ResultsGroup.size(); n++) {
#ifdef _DEBUG
				mexPrintf("in getLoadedDataAndClear() loop, n=%d\n", n);
				mexEvalString("pause(0.2);");
#endif
				const mxArray* thisRes = _ResultsGroup.getConstArray(n);
#ifdef _DEBUG
				mexPrintf("thisRes=%p\n", thisRes);
				mexPrintf("_ResPtrs[%d]=%p\n", n, _ResPtrs[n]);
				mexPrintf("_ResultsPtrs[%d]=%p\n", n, _ResultsPtrs[n]);

				mexEvalString("pause(0.2);");

				mexPrintf("classID: %s\n", mxGetClassName(thisRes));
				mexEvalString("pause(0.2);");
#endif
				mxArray* nonConstCopy = mxDuplicateArray(thisRes);
#ifdef _DEBUG
				mexPrintf("nonConstCopy=%p\n", nonConstCopy);
				mexEvalString("pause(0.2);");
#endif
#ifdef _DEBUG
				mexPrintf("out.getmxarray()=%p\n", out.getmxarray());
				mexEvalString("pause(0.2);");
#endif
				mxSetCell(out.getmxarray(), n, nonConstCopy);
			}

#ifdef _DEBUG
			mexPrintf("about to _ResultsGroup.clear();\n");
			mexEvalString("pause(0.2);");
#endif
			_ResultsGroup.clear();
#ifdef _DEBUG
			_ResPtrs.clear();
			_ResultsPtrs.clear();
			mexPrintf("cleared _ResultsGroup.size()=%d\n",_ResultsGroup.size());
			mexEvalString("pause(0.2);");
#endif
			return out;
		}

		/// true if there was an error
		virtual bool wasErrorThrown() const {
			return ErrorFlag;
		}

		/// Clears the error pointer and error flag.
		/// After calling ErroFlag=false and LastError=nullptr
		virtual void clearError() {
			ErrorFlag = false;
			std::lock_guard<std::mutex> lock(LastErrorMutex);
			LastError = nullptr;
		}

		///check for errors thrown within the processing loop
		///returns exception_ptr to the thrown exception
		///if there aren't any exceptions, returns nullptr
		virtual std::exception_ptr getError() const {
			//read exception flags
			//see discussion here: https://stackoverflow.com/questions/41288428/is-stdexception-ptr-thread-safe
			if (ErrorFlag) { //yes, exception was thrown
				return LastError;
			}
			else {//no exception, return nullptr
				return nullptr;
			}
		}
	};

	///////////
	// Mex Interface for AsyncReader

	//! implement mexInterface for AsyncMxFileReader
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of AsyncMxFileReader*/
	class AsyncMxFileReaderInterface : public SessionManager::mexInterface<ObjType, ObjManager> {
		typedef SessionManager::mexInterface<ObjType, ObjManager> ParentType;
	protected:
		void getCompressedSize(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateDoubleScalar(ParentType::getObjectPtr(nrhs, prhs)->getCompressedSize());
		}
		void getPositionInFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateDoubleScalar(ParentType::getObjectPtr(nrhs, prhs)->getPositionInFile());
		}
		void loadProgress(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateDoubleScalar(ParentType::getObjectPtr(nrhs, prhs)->loadProgress());
		}
		void filepath(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateString(ParentType::getObjectPtr(nrhs, prhs)->filepath().c_str());
		}
		void isEOF(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->isEOF());
		}
		void threadRunning(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->threadRunning());
		}
		void openFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			if (nrhs < 2) {
				throw(extras::stacktrace_error("AsyncMxFileReaderInterface::openFile() required 2 inputs"));
			}
			ParentType::getObjectPtr(nrhs, prhs)->openFile(extras::cmex::getstring(prhs[1]));
		}
		void closeFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->closeFile();
		}
		void loadData(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->loadData();
		}
		void resume(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->resume();
		}
		void pause(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->pause();
		}
		void cancel(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->cancel();
		}
		void numberOfArraysLoaded(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateDoubleScalar(ParentType::getObjectPtr(nrhs, prhs)->numberOfArraysLoaded());
		}
		void getLoadedData(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = ParentType::getObjectPtr(nrhs, prhs)->getLoadedData();
		}
		void getLoadedDataAndClear(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = ParentType::getObjectPtr(nrhs, prhs)->getLoadedDataAndClear();
		}
		void wasErrorThrown(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->wasErrorThrown());
		}
		void clearError(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearError();
		}
		void getError(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			std::exception_ptr err = ParentType::getObjectPtr(nrhs, prhs)->getError();

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
		void isFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool isopen = ParentType::getObjectPtr(nrhs, prhs)->isFileOpen();
			plhs[0] = mxCreateLogicalScalar(isopen);
		}
	public:
		AsyncMxFileReaderInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("isFileOpen", std::bind(&AsyncMxFileReaderInterface::isFileOpen, this, _1, _2, _3, _4));
			ParentType::addFunction("getCompressedSize", std::bind(&AsyncMxFileReaderInterface::getCompressedSize, this, _1, _2, _3, _4));
			ParentType::addFunction("getPositionInFile", std::bind(&AsyncMxFileReaderInterface::getPositionInFile, this, _1, _2, _3, _4));
			ParentType::addFunction("loadProgress", std::bind(&AsyncMxFileReaderInterface::loadProgress, this, _1, _2, _3, _4));
			ParentType::addFunction("filepath", std::bind(&AsyncMxFileReaderInterface::filepath, this, _1, _2, _3, _4));
			ParentType::addFunction("isEOF", std::bind(&AsyncMxFileReaderInterface::isEOF, this, _1, _2, _3, _4));
			ParentType::addFunction("threadRunning", std::bind(&AsyncMxFileReaderInterface::threadRunning, this, _1, _2, _3, _4));
			ParentType::addFunction("openFile", std::bind(&AsyncMxFileReaderInterface::openFile, this, _1, _2, _3, _4));
			ParentType::addFunction("closeFile", std::bind(&AsyncMxFileReaderInterface::closeFile, this, _1, _2, _3, _4));
			ParentType::addFunction("loadData", std::bind(&AsyncMxFileReaderInterface::loadData, this, _1, _2, _3, _4));
			ParentType::addFunction("resume", std::bind(&AsyncMxFileReaderInterface::resume, this, _1, _2, _3, _4));
			ParentType::addFunction("pause", std::bind(&AsyncMxFileReaderInterface::pause, this, _1, _2, _3, _4));
			ParentType::addFunction("cancel", std::bind(&AsyncMxFileReaderInterface::cancel, this, _1, _2, _3, _4));
			ParentType::addFunction("numberOfArraysLoaded", std::bind(&AsyncMxFileReaderInterface::numberOfArraysLoaded, this, _1, _2, _3, _4));
			ParentType::addFunction("getLoadedData", std::bind(&AsyncMxFileReaderInterface::getLoadedData, this, _1, _2, _3, _4));
			ParentType::addFunction("getLoadedDataAndClear", std::bind(&AsyncMxFileReaderInterface::getLoadedDataAndClear, this, _1, _2, _3, _4));
			ParentType::addFunction("wasErrorThrown", std::bind(&AsyncMxFileReaderInterface::wasErrorThrown, this, _1, _2, _3, _4));
			ParentType::addFunction("clearError", std::bind(&AsyncMxFileReaderInterface::clearError, this, _1, _2, _3, _4));
			ParentType::addFunction("getError", std::bind(&AsyncMxFileReaderInterface::getError, this, _1, _2, _3, _4));
		}
	};
}}