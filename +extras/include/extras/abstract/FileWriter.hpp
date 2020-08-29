/*----------------------------------------------------------------------
Copyright 2020, Daniel T. Kovari
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#include <string>

#include <extras/SessionManager/mexInterface.hpp>

namespace extras {namespace abstract{

	/** Abstract FileHandler Base Class
	* This class defines the interface of all file reader/writerer classes, it 
	*/
	class FileHandler {
	protected:
		std::string _filename;
	public:



		virtual void fopen(const std::string&, ) = 0; //open file for writing, throws an error if things aren't working
		virtual void openFile(size_t nrhs, const mxArray** prhs) = 0; //open file using matlab arguments, throws an error if things aren't working
		virtual int closeFile() = 0; //close file

		virtual bool isFileOpen() const = 0; //return true if file is open, otherwise return false.
		virtual const std::string& filepath() const = 0;//! returns copy of filepath string, returns empty string if file is not open.
	};


}}