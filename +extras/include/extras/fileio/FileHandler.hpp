/*----------------------------------------------------------------------
Copyright 2020, Daniel T. Kovari
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#include <string>
#include <cstring>
#include <stdexcept>

#include <extras/SessionManager/mexInterface.hpp>


namespace extras {namespace fileio {

	/** FileHandler Base Class
	* This class defines the interface of all file reader/writer classes.
	*/
	class FileHandler {
	protected:
		std::string _filename;
		FILE* _pFile;
		std::string _mode;
		bool _isopen = false;
	public:
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
			_pFile = fopen(filename, mode);
			if (_pFile == nullptr) {
				throw(std::runtime_error(std::string("FileHandler::fopen() Could not open file:") + std::string(filename)));
			}
		}

		/**Close file
		* return output of fclose
		* Upon return, file will be closed, file pointer will be set to null, and filename will be cleared
		*/
		virtual int closeFile() {
			int res = fclose(_pFile);
			_pFile = nullptr;
			_isopen = false;
			_filename.clear();
			_mode.clear();

			return res;
		}

		virtual bool isFileOpen() const { return _isopen; }//return true if file is open, otherwise return false.
		virtual std::string filepath() const { return _filename; }//! returns a copy of the filepath string, returns empty string if file is not open.

		////////////////////////////
		// 
		virtual ~FileHandler() {
			closeFile();
		}
	};


}}