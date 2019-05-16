/*----------------------------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari & James Merrill, Emory University
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#define NOMINMAX
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <mex.h>
#include <list>
#include <string>
#include <vector>
#include <mutex>
#include <cstring>

#include <extras/cmex/mxobject.hpp>
#include <extras/SessionManager/mexInterface.hpp>
#include <extras/string_extras.hpp>
#include <extras/async/AsyncProcessor.hpp>

#include <extras/stacktrace_error.hpp>

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
#include <zlib.h>
/********************************************/

///////////////////////////////////////////////
// MxFileWriter
namespace extras {namespace mxfile {

	class MxFileWriter; //forward declaration

	// Data Structure
	struct SerialData {
		size_t nbytes; // number of bytes of data to write
		const mxArray* data;  //the mxArray to be written to disk
	};

	//! Flattens list of mxArray*s into a serialized list
	//! Cells and struct are decomposed into a serialized list of numeric/char type arrays
	//! resulting serialized data is returned as a list of "SerialData" a simple struct containing a
	//! const mxArray* and nbytes, the size the struct will be on the disk (when uncompressed)
	std::list<SerialData> Serialize(size_t nrhs, const mxArray** prhs) {
		std::list<SerialData> out;
		for (size_t i = 0; i < nrhs; i++) {
			SerialData l;

			if (prhs[i] == nullptr) { //passed nullptr, interpret as empty numeric array
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + sizeof(uint8_t) + sizeof(uint8_t); //type, ndims, isComplex, isInterleaved,
				l.nbytes += 0; //mxGetNumberOfDimensions(prhs[i]) * sizeof(size_t) + mxGetNumberOfElements(prhs[i]) * mxGetElementSize(prhs[i]) * (1 + mxIsComplex(prhs[i])); //  dims, data
				l.data = prhs[i];
				out.push_back(l);
				continue;
			}

			switch (mxGetClassID(prhs[i])) {
			case mxCELL_CLASS:
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + sizeof(size_t) * mxGetNumberOfDimensions(prhs[i]);
				//type, ndim, dims
				l.data = prhs[i];
				out.push_back(l);
				for (size_t j = 0; j < mxGetNumberOfElements(prhs[i]); ++j) {

					const mxArray * c = mxGetCell(prhs[i], j);

					out.splice(out.end(), Serialize(1, &c));

				}
				break;
			case mxSTRUCT_CLASS: {
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + mxGetNumberOfDimensions(prhs[i]) + sizeof(std::string);
				for (size_t j = 0; j < mxGetNumberOfFields(prhs[i]); ++j) {
					l.nbytes += sizeof(char) * std::string(mxGetFieldNameByNumber(prhs[i], j)).length();
				}
				//type, ndims, dims, nfields, field names
				l.data = prhs[i];
				out.push_back(l);
				for (size_t j = 0; j < mxGetNumberOfElements(prhs[i]); ++j) {
					for (size_t k = 0; k < mxGetNumberOfFields(prhs[i]); ++k) {
						const mxArray* c = mxGetFieldByNumber(prhs[i], j, k);
						out.splice(out.end(), Serialize(1, &c)); //unfold contents of fields
					}
				}
			}
			break;
			default:
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + sizeof(uint8_t) + sizeof(uint8_t); //type, ndims, isComplex, isInterleaved,
				l.nbytes += mxGetNumberOfDimensions(prhs[i]) * sizeof(size_t) + mxGetNumberOfElements(prhs[i]) * mxGetElementSize(prhs[i]) * (1 + mxIsComplex(prhs[i])); //  dims, data
				l.data = prhs[i];
				out.push_back(l);
			}
		}
		return out;
	}

	class SimpleWriter {
	protected:
		FILE * _fp = nullptr;
	public:
		SimpleWriter() : _fp(nullptr) {};
		//SimpleWriter(const char* filepath) : _fp(nullptr) { open(filepath); };
		~SimpleWriter() {
			close();
		}
		virtual void close() {
			if (_fp != nullptr) {
				fclose(_fp);
				_fp = nullptr;
			}
		}
		virtual void open(const char* filepath) {
			if (_fp != nullptr) {
				fclose(_fp);
				_fp = nullptr;
			}
			_fp = fopen(filepath, "wb");
			if (_fp == NULL) {
				throw(extras::stacktrace_error(std::string("open(): returned null, file:'") + std::string(filepath) + std::string("' could not be opened.")));
			}
		}

		virtual void flush() {};

		//! write data to file pointer
		//! Input:
		//!		data: pointer to data to write
		//!		nbytes: number of bytes to write
		//! Return: number of bytes written
		virtual size_t write(const void* data, size_t nbytes) {
			return fwrite(data, nbytes, 1, _fp);
		}

		virtual bool isopen() const {
			return _fp == nullptr;
		}
	};

	class SimpleGzWriter : public SimpleWriter {
		friend class MxFileWriter;
	protected:
		gzFile _fp=nullptr; //zlib file pointer
		//char* _data_buffer = nullptr;
		//size_t _data_buffer_sz = 16384;//16kB
		//size_t _data_buffer_pos = 0;

	public:
		SimpleGzWriter() : _fp(nullptr) {
			//_data_buffer = (char*)malloc(_data_buffer_sz);
		};

		//SimpleGzWriter(const char* filepath) : _fp(nullptr) { open(filepath); };

		~SimpleGzWriter() {
			close();
			//free(_data_buffer);
		}

		virtual bool isopen() const {
			return _fp != nullptr;
		}

		virtual void close() {
			if (_fp != nullptr) {
				flush();
				gzclose(_fp);
				_fp = nullptr;
			}
		}

		virtual void open(const char* filepath) {
			if (_fp != nullptr) {
				gzclose(_fp);
				_fp = nullptr;
			}
			_fp = gzopen(filepath, "wb");
			if (_fp == NULL) {
				throw(extras::stacktrace_error(std::string("gzopen(): returned null, file:'") + std::string(filepath) + std::string("' could not be opened.")));
			}
			gzbuffer(_fp, 262144);
			gzsetparams(_fp, 9, Z_DEFAULT_STRATEGY);
		}

		virtual void flush() {
			/*if (_data_buffer_pos > 0 && _fp != NULL) {
				gzwrite(_fp, _data_buffer, _data_buffer_pos);
				_data_buffer_pos = 0;
			}*/
		}

		//! write data to file pointer
		//! Input:
		//!		data: pointer to data to write
		//!		nbytes: number of bytes to write
		//! Return: number of bytes written
		virtual size_t write(const void* data, size_t nbytes) {
			return gzwrite(_fp, data, nbytes);
			/*
			if (nbytes >= _data_buffer_sz) { //writing big data, write the rest of the buffer, then write the data
				flush();
				return gzwrite(_fp, data, nbytes);
			}
			else { //writing small data, use the buffer
				size_t rem = _data_buffer_sz - _data_buffer_pos;

				size_t ncpy = std::min(rem, nbytes);
				memcpy(&_data_buffer[_data_buffer_pos], data, ncpy);
				_data_buffer_pos += ncpy;
				if (_data_buffer_pos == _data_buffer_sz) {
					gzwrite(_fp, _data_buffer, _data_buffer_sz);
					_data_buffer_pos = 0;
				}
				if (nbytes > ncpy) {
					memcpy(_data_buffer, &((char*)data)[ncpy], nbytes - ncpy);
					_data_buffer_pos += nbytes - ncpy;
				}

			}
			*/
		}
	};


	//! Loop over all arrays in the serialized list and write to FP
	void writeList(const std::list<SerialData>& dataList, SimpleWriter& FP) {
		for (auto& thisData : dataList) { //loop over all items in the list
			const mxArray* thisArray = thisData.data;
			uint8_t type; //= mxGetClassID(thisArray);
			size_t ndims; //= mxGetNumberOfDimensions(thisArray);
			void* dims = nullptr;

			if (thisArray == nullptr) { //passed nullptr, interpret as empty numeric array
				type = mxDOUBLE_CLASS;
				ndims = 0;
			}
			else {
				type = mxGetClassID(thisArray);
				ndims = mxGetNumberOfDimensions(thisArray);
				dims = (void*)mxGetDimensions(thisArray);
			}

			//// write header

			FP.write(&type, 1);//write type byte
			FP.write(&ndims, sizeof(size_t));//write ndim
			FP.write(dims, sizeof(size_t)*ndims);//write dims

			// Write datatype specific info
			switch (type)
			{
			case mxCELL_CLASS:
				//nothing else to write
				break;
			case mxSTRUCT_CLASS:
			{
				//write number of field names
				size_t nfields = mxGetNumberOfFields(thisArray);
				FP.write(&nfields, sizeof(size_t));

				////////////
				// FOR EACH FIELD
				//write length of fieldname and fieldnames
				for (size_t f = 0; f < nfields; ++f) {
					const char* fieldname = mxGetFieldNameByNumber(thisArray, f);
					size_t len = strlen(fieldname) + 1; // length including null terminator

					//write length of name
					FP.write(&len, sizeof(size_t));

					//write fieldname, including null terminator
					FP.write(fieldname, len * sizeof(char));
				}
			}
			break;
			default: //all other types
			{
				if (thisArray == nullptr) { //passed nullptr, interpret as empty numeric array
					//write complex
					uint8_t isComplex = 0;
					FP.write(&isComplex, sizeof(uint8_t));

					//write interleaved flag
					uint8_t interFlag = 0; //default to not interleaved complex data
#if MX_HAS_INTERLEAVED_COMPLEX
					interFlag = 1;
#endif
					FP.write(&interFlag, sizeof(uint8_t));

					//Write data
						//nothing to do since there is no data
				}
				else
				{
					//write complex
					uint8_t isComplex = (uint8_t)mxIsComplex(thisArray);
					FP.write(&isComplex, sizeof(uint8_t));

					//write interleaved flag
					uint8_t interFlag = 0; //default to not interleaved complex data
#if MX_HAS_INTERLEAVED_COMPLEX
					interFlag = 1;
#endif
					FP.write(&interFlag, sizeof(uint8_t));

					//Write data
					size_t numel = mxGetNumberOfElements(thisArray);
					size_t elsz = mxGetElementSize(thisArray);
					if (!isComplex) { //not complex, simple write
						FP.write(mxGetData(thisArray), numel*elsz);
					}
					else { //is complex
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved data, size is 2x so regular copy works fine
						FP.write(mxGetData(thisArray), numel*elsz);
#else //not interleaved, need to write imag data explicitly
						FP.write(mxGetData(thisArray), numel*elsz);
						FP.write(mxGetImagData(thisArray), numel*elsz);
#endif
					}
				}
			}
			}
			
			// force file write
			//FP.flush();
		}
	}

	/**
		* Class which handles writing MxFile data
	*/
	class MxFileWriter {
	protected:
		std::mutex _WPmutex; //mutex protecting _WritePointer
		SimpleGzWriter _WritePointer; //pointer class for gzFile, proteced by locks using _WPmutex
		std::string _filepath;

		//! automatically add ".mxf.gz" file extension if not included
		std::string validateFileExt(std::string fpth) {
			//look for ".mxf.gz" at end of file
			std::string fpth_lower = extras::tolower(fpth);
			const char* fpth_c = fpth_lower.c_str();
			const char* p_ext = strstr(fpth_c, ".mxf.gz");
			if (p_ext != nullptr) { //found in fpth, make sure it's at the end
				size_t loc = p_ext - fpth_c; //location in the string
				size_t back = fpth.size() - loc; //loc from back
				if (back == 7) { //found at back, just return fpth
					return fpth;
				}
			}
			//didn't find ".mxf.gz", look for ".mxf"
			p_ext = strstr(fpth_c, ".mxf");
			if (p_ext != nullptr) { //found in fpth, make sure it's at the end
				size_t loc = p_ext - fpth_c; //location in the string
				size_t back = fpth.size() - loc; //loc from back
				if (back == 7) { //found at back, add ".gz"
					fpth += ".gz";
					return fpth;
				}
			}
			//didn't find ".mxf" add ".mxf.gz"
			fpth += ".mxf.gz";
			return fpth;
		}
	public:

		//! return true if file is open
		bool isFileOpen() const {
			return _WritePointer.isopen();
		}

		//! default constructor
		MxFileWriter() {};

		//! destructor
		//! close the writePointer
		virtual ~MxFileWriter() {
			closeFile();
		}

		//! close the file
		virtual void closeFile() {
			if (isFileOpen()) {
				std::lock_guard<std::mutex> lock(_WPmutex);
				_WritePointer.close();
			}
		}

		//! open specified file for writing
		//! automatically adds ".mxf.gz" file extension if not included
		virtual void openFile(std::string fpth) {
			closeFile();
			std::lock_guard<std::mutex> lock(_WPmutex);
			auto fp_ext = validateFileExt(fpth);
			_WritePointer.open(fp_ext.c_str());
			_filepath = fp_ext;
		}

		//!open file for writing (using MATLAB args)
		//! automatically add ".mxf.gz" file extension if not included
		virtual void openFile(size_t nrhs, const mxArray** prhs) {
			if (nrhs < 1) {
				throw(extras::stacktrace_error("MxFileWriter::openWriter() expected one argument"));
			}
			openFile(extras::cmex::getstring(prhs[0]));
		}

		//! returns copy of filepath string
		std::string filepath() const {
			return _filepath;
		}

		//! write the matlab arrays to the file
		virtual void writeArrays(size_t nrhs, const mxArray** prhs) {
			if (!isFileOpen()) {
				throw(extras::stacktrace_error(std::string("MxFileWriter::writeArrays() file:'") + _filepath + std::string("' is not open.")));
			}
			std::lock_guard<std::mutex> lock(_WPmutex);
			writeList(Serialize(nrhs, prhs), _WritePointer);
		}
	};

	//! implement mexInterface for MxFileWriter
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of MxFileWriter*/
	class MxFileWriterInterface : public SessionManager::mexInterface<ObjType, ObjManager> {
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
		void writeArrays(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->writeArrays(nrhs - 1, &prhs[1]);
		}
		void isFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool isopen = ParentType::getObjectPtr(nrhs, prhs)->isFileOpen();
			plhs[0] = mxCreateLogicalScalar(isopen);
		}
	public:
		MxFileWriterInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("openFile", std::bind(&MxFileWriterInterface::openFile, this, _1, _2, _3, _4));
			ParentType::addFunction("closeFile", std::bind(&MxFileWriterInterface::closeFile, this, _1, _2, _3, _4));
			ParentType::addFunction("filepath", std::bind(&MxFileWriterInterface::filepath, this, _1, _2, _3, _4));
			ParentType::addFunction("writeArrays", std::bind(&MxFileWriterInterface::writeArrays, this, _1, _2, _3, _4));
			ParentType::addFunction("isFileOpen", std::bind(&MxFileWriterInterface::isFileOpen, this, _1, _2, _3, _4));
		}
	};

}}

///////////////////////////////////////////
// ASYNC WRITER
namespace extras {namespace mxfile {
	///////////////////////////////////////////////
	// Async Writer
	class AsyncMxFileWriter : public MxFileWriter, virtual public extras::async::AsyncProcessor {
	protected:

		//! redefine StartProcessor to check if file is open
		virtual void StartProcessor() {
			if (!isFileOpen()) {
				throw(extras::stacktrace_error(std::string("AsyncMxFileWriter::StartProcessor() file:'") + _filepath + std::string("' is not open. Cannot start processor.")));
			}
			extras::async::AsyncProcessor::StartProcessor();
		}

		///Overloadable virtual method for Processing Tasks in the task list
		virtual cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) {
			//_AMFW_procced++;
			MxFileWriter::writeArrays(args.size(), args);
			return cmex::mxArrayGroup(); //return empty results array
		}

	public:
		//! destructor
		virtual ~AsyncMxFileWriter() {
			ProcessTasksAndEnd();
			closeFile();
		}

		//! default constructor
		AsyncMxFileWriter() {
			_firstTask = false;  //change first task flag, so that processor is not automatically started.
		}

		//! close the file
		//! stops processor before closing
		virtual void closeFile() {
			extras::async::AsyncProcessor::StopProcessor();
			MxFileWriter::closeFile();
		}

		//! open specified file for writing
		//! automatically adds ".mxf.gz" file extension if not included
		//! starts processor
		virtual void openFile(std::string fpth) {
			MxFileWriter::openFile(std::move(fpth));
			resume();
		}

		//! write the matlab arrays to the file
		virtual void writeArrays(size_t nrhs, const mxArray** prhs) {
			//_AMFW_pushed++;
			pushTask(nrhs, prhs);
		}

		//size_t get_AMFW_pushed() const { return _AMFW_pushed; }
		//size_t get_AMFW_procced() const { return _AMFW_procced; }
	};

	//! implement mexInterface for AsyncMxFileWriter
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of AsyncMxFileWriter*/
	class AsyncMxFileWriterInterface : public extras::async::AsyncMexInterface<ObjType, ObjManager> {
		typedef extras::async::AsyncMexInterface<ObjType, ObjManager> ParentType;
	protected:
		void openFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			if (nrhs < 2) {
				throw(extras::stacktrace_error("AsyncMxFileWriterInterface::openFile() two arguments required"));
			}
			ParentType::getObjectPtr(nrhs, prhs)->openFile(extras::cmex::getstring(prhs[1]));
		}
		void closeFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->closeFile();
		}
		void filepath(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			cmex::MxObject fpth = ParentType::getObjectPtr(nrhs, prhs)->filepath();
			plhs[0] = fpth;
		}
		void writeArrays(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->writeArrays(nrhs - 1, &prhs[1]);
		}
		void isFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			bool isopen = ParentType::getObjectPtr(nrhs, prhs)->isFileOpen();
			plhs[0] = mxCreateLogicalScalar(isopen);
		}
	public:
		AsyncMxFileWriterInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("openFile", std::bind(&AsyncMxFileWriterInterface::openFile, this, _1, _2, _3, _4));
			ParentType::addFunction("closeFile", std::bind(&AsyncMxFileWriterInterface::closeFile, this, _1, _2, _3, _4));
			ParentType::addFunction("filepath", std::bind(&AsyncMxFileWriterInterface::filepath, this, _1, _2, _3, _4));
			ParentType::addFunction("writeArrays", std::bind(&AsyncMxFileWriterInterface::writeArrays, this, _1, _2, _3, _4));
			ParentType::addFunction("isFileOpen", std::bind(&AsyncMxFileWriterInterface::isFileOpen, this, _1, _2, _3, _4));
		}
	};
}}