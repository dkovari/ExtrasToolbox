/*----------------------------------------------------------------------
Copyright 2020, Daniel T. Kovari
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#include <string>
#include <cstring>
#include <stdexcept>
#include <mutex>
#include <atomic>

#include <extras/cmex/MxStruct.hpp>
#include <extras/SessionManager/mexInterface.hpp>
#include <extras/string_extras.hpp>
#include <extras/async/ParamProcessor_nowriter.hpp>
#include <extras/strhash.h>


namespace extras { namespace fileio {
	/** AsyncFile - Class for reading/writing to files asyncronously
	*/
	class AsyncCSVWriter : virtual public extras::async::ParamProcessor_nowriter {
	protected:
		std::string _filename = "";
		FILE* _pFile = nullptr;;
		std::string _mode = "w";
		std::mutex _fileMutex;
		std::atomic_bool _isopen = false;

		/** Replaces AsyncProcessor::ProcessTask
		* This method defines how array itmes on the AsyncProcess::TaskList are processed.
		* Task items are expercted to be char array, or scalar numeric
		*
		* The output of ProcessTask() is a MxStruct specifying
		*	.function (mxchar) : String equal to "funciton_name"
		*	.return (mxarray): value returned by the function (e.g. int or bool)
		*	.ftell_before (int): position in file at start of operation
		*	.ftell_after (int): position in file after operation
		*
		* If this function runs into a problem, it should throw an error which will be caught by
		* the Task processing loop. The task processing loop should pause the processor and post
		* the error to the AsyncProcess error notification system.
		*/
		virtual extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& TaskArgs, std::shared_ptr<const extras::cmex::ParameterMxMap> Params) {

			if (!isFileOpen()) {
				throw(std::runtime_error("File is not open."));
			}

			cmex::MxStruct Result({ 1,1 }, { "ftell_before","ftell_after","warning" }); //create struct to hold result
			Result(0, "ftell_before") = ftell(_pFile); //get file position before we write

			for (size_t n = 0; n < TaskArgs.size(); n++) {
				if (!(mxIsNumeric(TaskArgs[n]) || mxIsChar(TaskArgs[n]))) {
					throw(std::runtime_error("only char or numeric array supported"));
				}


				if (mxIsNumeric(TaskArgs[n])){ //numeric
					if (!mxIsScalar(TaskArgs[n])) {
						throw(std::runtime_error("only scalar numeric supported"));
					}

					if (Params->isparameter("NumericFormat")) {
						fprintf(_pFile, cmex::getstring((*Params)["NumericFormat"]).c_str(), mxGetScalar(TaskArgs[n]));
					}
					else {
						fprintf(_pFile, "%G", mxGetScalar(TaskArgs[n]));
					}
				}
				else {//char
					if (Params->isparameter("StringFormat")) {
						fprintf(_pFile, cmex::getstring((*Params)["StringFormat"]).c_str(), cmex::getstring(TaskArgs[n]).c_str());
					}
					else {
						fprintf(_pFile, cmex::getstring(TaskArgs[n]).c_str());
					}
				}


				//print 
				if (n < TaskArgs.size() - 1) {
					fprintf(_pFile, ",");
				}
				else {
					fprintf(_pFile, "\n");
				}
			}
			fflush(_pFile); //flush file stream
			Result(0, "ftell_after") = ftell(_pFile); //get file position before we write

			return cmex::mxArrayGroup((mxArray*)Result);
		}
	public:
		/** Close file and destroy object
		*/
		virtual ~AsyncCSVWriter() {
			ProcessTasksAndEnd();
			closeFile();
		}

		/**open file, throws an error if things aren't working
		* expects the same syntax as standard fopen
		* if a file is already open, it will be closed first.
		*/
		virtual void openFile(const char* filename, const char* mode) {

			//close file if needed
			closeFile();

			//validate write mode
			size_t mode_sz = strlen(mode);
			if (mode_sz != strspn(mode, "rw+ba")) {
				throw(std::runtime_error(std::string("FileHandler::fopen() Invalid open mode: ") + std::string(mode)));
			}

			//open file
			std::lock_guard<std::mutex> lock(_fileMutex); //lock file
			_pFile = fopen(filename, mode);
			if (_pFile == nullptr) {
				throw(std::runtime_error(std::string("FileHandler::fopen() Could not open file:") + std::string(filename)));
			}

			_isopen = true;
			_filename = filename;
			_mode = mode;

		}

		/**Close file
		* return output of fclose
		* Upon return, file will be closed, file pointer will be set to null, and filename will be cleared
		*/
		virtual int closeFile() {
			std::lock_guard<std::mutex> lock(_fileMutex); //lock file
			int res = 0;
			if(_isopen && _pFile!=nullptr)
			{
				fflush(_pFile); //flush file stream
				res = fclose(_pFile);
				if (res != 0) {
					mexPrintf("Error closing file.\n");
				}
			}
				
			_pFile = nullptr;
			_isopen = false;
			_filename.clear();
			_mode.clear();

			return res;
		}

		virtual std::string fileAccessMode() const { return _mode; }//return file access mode

		virtual bool isFileOpen() const { return _isopen; }//return true if file is open, otherwise return false.
		virtual std::string filepath() const { return _filename; }//! returns a copy of the filepath string, returns empty string if file is not open.
	};


	// Extend the AsyncInterface
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of AsyncCSVWriter*/
	class AsyncCSVWriterInterface :public extras::async::ParamProcessor_nowriterInterface<ObjType, ObjManager> {
		typedef extras::async::ParamProcessor_nowriterInterface<ObjType, ObjManager> ParentType;
	protected:
		void openFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			if (nrhs < 2) {
				throw(std::runtime_error("No filename specified"));
			}
			std::string write_mode = "w";
			if (nrhs >= 3) {
				write_mode = cmex::getstring(prhs[2]);
			}
			ParentType::getObjectPtr(nrhs, prhs)->openFile(cmex::getstring(prhs[1]).c_str(), write_mode.c_str());
		}
		void closeFile(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateDoubleScalar(ParentType::getObjectPtr(nrhs, prhs)->closeFile());
		}

		void isFileOpen(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->isFileOpen());
		}

		void filepath(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = cmex::MxObject(ParentType::getObjectPtr(nrhs, prhs)->filepath());
		}

		void fileAccessMode(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = cmex::MxObject(ParentType::getObjectPtr(nrhs, prhs)->fileAccessMode());
		}

	public:
		AsyncCSVWriterInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("openFile", std::bind(&AsyncCSVWriterInterface::openFile, this, _1, _2, _3, _4));
			ParentType::addFunction("closeFile", std::bind(&AsyncCSVWriterInterface::closeFile, this, _1, _2, _3, _4));
			ParentType::addFunction("isFileOpen", std::bind(&AsyncCSVWriterInterface::isFileOpen, this, _1, _2, _3, _4));
			ParentType::addFunction("filepath", std::bind(&AsyncCSVWriterInterface::filepath, this, _1, _2, _3, _4));
			ParentType::addFunction("fileAccessMode", std::bind(&AsyncCSVWriterInterface::fileAccessMode, this, _1, _2, _3, _4));
		}
	};

}}