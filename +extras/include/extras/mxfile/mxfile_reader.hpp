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
	};

	//! Read next mxArray from the file
	extras::cmex::MxObject readNext(FILE_ReadPointer& FP) {
		extras::cmex::MxObject out;

		// read type
		mxClassID thisClassID;
		uint8_t tmp;
		if (FP.read(&tmp, 1) < 1) {
			if (FP.eof()) {
				throw("readNext(): at EOF");
				//return out;//return empty array;
			}
			else {
				throw(FP.error_msg());
			}
		}
		thisClassID = (mxClassID)tmp;

		//read ndim
		size_t ndim;
		if (FP.read(&ndim, sizeof(size_t)) < sizeof(size_t)) {
			if (FP.eof()) {
				throw("reached end of file while reading ndim");
			}
			else {
				throw(FP.error_msg());
			}
		}

		//read dims
		std::vector<size_t> dims(ndim);
		if (FP.read(dims.data(), sizeof(size_t)*ndim) < sizeof(size_t)*ndim) {
			if (FP.eof()) {
				throw("reached end of file while reading dims");
			}
			else {
				throw(FP.error_msg());
			}
		}

		//create array to hold data
		switch (thisClassID) {
			case mxCELL_CLASS:
			{
				out.own(mxCreateCellArray(ndim, dims.data()));
				for (size_t n = 0; n < out.numel(); n++) {
					mxSetCell(out.getmxarray(), n, readNext(FP));
				}
			}
				break;
			case mxSTRUCT_CLASS:
			{
				//get nfields
				size_t nfields;
				if (FP.read(&nfields, sizeof(size_t)) < sizeof(size_t)) {
					if (FP.eof()) {
						throw("reached end of file while reading nfields");
					}
					else {
						throw(FP.error_msg());
					}
				}

				std::vector<std::vector<char>> fieldnames(nfields); //vector of strings in which names are stored
				std::vector<const char*> fnames(nfields); //vector of const char* used to call mxCreateStruct...
				for (size_t f = 0; f < nfields; f++) { //loop and read names
					size_t len; //length of fieldname including null terminator
					if (FP.read(&len, sizeof(size_t)) < sizeof(size_t)) {
						if (FP.eof()) {
							throw("reached end of file while reading length of fieldname");
						}
						else {
							throw(FP.error_msg());
						}
					}

					fieldnames[f].resize(len); //read fieldname, including null terminator
					if (FP.read(fieldnames[f].data(), len) < len) {
						if (FP.eof()) {
							throw("reached end of file while reading lfieldname");
						}
						else {
							throw(FP.error_msg());
						}
					}
					fnames[f] = fieldnames[f].data(); //set pointer array
				}

				out.own(mxCreateStructArray(ndim, dims.data(), nfields, fnames.data()));

				/// Loop and read fields
				for (size_t j = 0; j < out.numel(); ++j) {
					for (size_t k = 0; k < nfields; ++k) {
						mxSetFieldByNumber(out.getmxarray(), j, k, readNext(FP));
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
						throw("reached end of file while reading isComplex");
					}
					else {
						throw(FP.error_msg());
					}
				}

				//read interleaved
				uint8_t interFlag;
				if (FP.read(&interFlag, 1) < 1) {
					if (FP.eof()) {
						throw("reached end of file while reading interFlag");
					}
					else {
						throw(FP.error_msg());
					}
				}

				if (thisClassID == mxCHAR_CLASS) { //handle char
					out.own(mxCreateCharArray(ndim, dims.data()));
					size_t elsz = out.elementBytes();
					if (FP.read(mxGetData(out.getmxarray()), elsz*out.numel()) < elsz*out.numel()) {
						if (FP.eof()) {
							throw("reached end of file while reading char data");
						}
						else {
							throw(FP.error_msg());
						}
					}
				}
				else if (!isComplex) { //real
					out.own(mxCreateNumericArray(ndim, dims.data(), thisClassID, mxREAL));
					size_t elsz = out.elementBytes();
					if (FP.read(mxGetData(out.getmxarray()), elsz*out.numel()) < elsz*out.numel()) {
						if (FP.eof()) {
							throw("reached end of file while reading real numeric data");
						}
						else {
							throw(FP.error_msg());
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
								throw("reached end of file while reading interleaved complex numeric data");
							}
							else {
								mxFree(data); //cleanup out memory mess
								throw(FP.error_msg());
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
								throw("reached end of file while reading non-interleaved real data for complex numeric");
							}
							else {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw(FP.error_msg());
							}
						}
						if (FP.read(imagData, numel*elsz_real) < numel*elsz_real) { //read imag
							if (FP.eof()) {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw("reached end of file while reading non-interleaved imag data for complex numeric");
							}
							else {
								mxFree(realData); //cleanup out memory mess
								mxFree(imagData); //cleanup out memory mess
								throw(FP.error_msg());
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

		//! open specified file for writing
		void openFile(std::string fpth) {
			closeFile();
			gzFile gzf = gzopen(fpth.c_str(), "rb");
			if (gzf == NULL) {
				throw(std::runtime_error(std::string("MxFileReader::openFile(): returned null, file:'") + std::string(fpth) + std::string("' could not be opened.")));
			}
			std::lock_guard<std::mutex> lock(_RPmutex);
			_ReadPointer = GZFILE_ReadPointer(gzf);
			_filepath = fpth;
		}

		//!open file for writing (using MATLAB args)
		void openFile(size_t nrhs, const mxArray** prhs) {
			if (nrhs < 1) {
				throw("MxFileReader::openWriter() expected one argument");
			}
			openFile(extras::cmex::getstring(prhs[0]));
		}

		//! attempt to read next array from file
		extras::cmex::MxObject readNextArray() {
			if (!isFileOpen()) {
				throw(std::runtime_error(std::string("MxFileReader::readNextArray() file:'") + _filepath + std::string("' is not open.")));
			}
			if (isEOF()) {
				throw(std::runtime_error(std::string("MxFileReader::readNextArray() File:'") + _filepath + std::string("'. Reached End of File.")));
			}
			return readNext(_ReadPointer);
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
			plhs[0] = ParentType::getObjectPtr(nrhs, prhs)->readNextArray();
		}
		void isFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool isopen = ParentType::getObjectPtr(nrhs, prhs)->isFileOpen();
			plhs[0] = mxCreateLogicalScalar(isopen);
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
		}
	};

}}