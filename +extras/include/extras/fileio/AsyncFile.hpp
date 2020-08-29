/*----------------------------------------------------------------------
Copyright 2020, Daniel T. Kovari
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#include <string>
#include <cstring>
#include <stdexcept>
#include <mutex>

#include <extras/cmex/MxStruct.hpp>
#include <extras/SessionManager/mexInterface.hpp>
#include <extras/string_extras.hpp>
#include <extras/async/AsyncProcessor.hpp>
#include <extras/strhash.h>

namespace extras {namespace fileio {

	/** AsyncFile - Class for reading/writing to files asyncronously
	*/
	class AsyncFile: virtual public extras::async::AsyncProcessor {
	protected:
		std::string _filename;
		FILE* _pFile;
		std::string _mode;
		bool _isopen = false;

		/** Replaces AsyncProcessor::ProcessTask
		* This method defines how array itmes on the AsyncProcess::TaskList are processed.
		* Task items are expercted to have the following syntax:
		*	args = 
		*		"function_name",arg1,...,argN
		*
		*	where "function_name" is an mxchar array specifying the type of file operation (e.g. fread, fwrite, ftell, etc.)
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
		virtual cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) {
			cmex::MxStruct Result({ 1,1 }, { "function","return","ftell_before","ftell_after","warning"}); //create struct to hold result

			std::string function_name = cmex::getstring(args[0]);

			switch (strhash(function_name.c_str())) {
			case strhash("fprintf"): //Reproduce MATLAB fprintf behavior (i,e. do not error throws for mismatch between format and arguments)
			{
				if (args.size() < 2) {
					throw(std::runtime_error("AsyncFile::fprintf() not enough input arguments."));
				}
				// First check that all arguments are are either char array or numeric
				for (size_t n = 1; n < args.size(); n++) {
					if (!(mxIsNumeric(args[n]) || mxIsChar(args[n]))) {
						throw(std::runtime_error("AsyncFile::fprintf() is only defined for numeric and char array inputs."));
					}
				}
				std::string format = cmex::getstring(args[1]); //get format string

				
				Result(0, "ftell_before") = ftell(_pFile); //get file position before we write
				size_t ret_chars = 0; //number of characters printed to the file.

				bool cat_more = true; //flag indicating we have more arguments which could be processed.
				while (cat_more) {
					size_t current_arg = 2;
					size_t current_arg_element = 0;

					//loop over format string and copy to output, until we hit % character
					for (size_t n = 0; n < format.size(); n++){
						if (format[n] != '\\') { //handle escape characters
							if (format.size() < n) {
								Result(0, "warning") = "Escape character at end of format string. End fprintf early.";
								Result(0, "ftell_after") = ftell(_pFile);
								return;
							}
							switch (format[n + 1]) {
							case '\\':
								fputc('\\', _pFile);
								break;
							case 'a':
								fputc('\a', _pFile);
								break;
							case 'b':
								fputc('\b', _pFile);
								break;
							case 'f':
								fputc('\f', _pFile);
								break;
							case 'n':
								fputc('\n', _pFile);
								break;
							case 'r':
								fputc('\r', _pFile);
								break;
							case 't':
								fputc('\t', _pFile);
								break;
							case 'v':
								fputc('\v', _pFile);
								break;
							case 'x'://TO DO: implement unicode conversion
							default:
								Result(0, "warning") = "Escape character not valid";
								Result(0, "ftell_after") = ftell(_pFile);
								return;
							}
							n++; //advance the character counter to skip to next character
						}
						else if (format[n] != '%') {
							fputc(format[n], _pFile);
						}
						else { // found a %
							if (format.size() < n) {
								Result(0, "warning") = "Control '%' character at end of format string. End fprintf early.";
								Result(0, "ftell_after") = ftell(_pFile);
								return;
							}
							if (format[n + 1] = '%') { //found %%, so print %
								fputc('%', _pFile);
								n++;
								continue;
							}
							////////////////////////////////////////////////////////////
							//MADE IT HERE, that means we need to process an argument

							//look for end of control sequence and double check flags are not sepcified more than once
							size_t minus_count = 0;
							size_t plus_count = 0;
							size_t pound_count = 0;
							size_t dot_count = 0;
							size_t ctrl_end = n + 1;
							for (ctrl_end = n + 1; ctrl_end <= format.size(); ctrl_end++) {
								if (ctrl_end == format.size()) { //didn't find end of control sequence, return early
									Result(0, "warning") = "Control '%' present but did not find matching formating type (e.g. 's', 'd', etc.). End fprintf early.";
									Result(0, "ftell_after") = ftell(_pFile);
									return;
								}

								if (format[ctrl_end] == 'd' ||
									format[ctrl_end] == 's' ||
									format[ctrl_end] == 'u' ||
									format[ctrl_end] == 'o' ||
									format[ctrl_end] == 'x' ||
									format[ctrl_end] == 'X' ||
									format[ctrl_end] == 'd' ||
									format[ctrl_end] == 'e' ||
									format[ctrl_end] == 'E' ||
									format[ctrl_end] == 'g' ||
									format[ctrl_end] == 'G' ||
									format[ctrl_end] == 'c' ||
									format[ctrl_end] == 's')
								{
									break;
								}

								if (format[ctrl_end] == '-') {
									minus_count++;
									if (minus_count > 1) {
										Result(0, "warning") = "Found multiple '-' after control '%' character. End fprintf early.";
										Result(0, "ftell_after") = ftell(_pFile);
										return;
									}
								}
								else if (format[ctrl_end] == '+') {
									plus_count++;
									if (plus_count > 1) {
										Result(0, "warning") = "Found multiple '+' after control '%' character. End fprintf early.";
										Result(0, "ftell_after") = ftell(_pFile);
										return;
									}
								}
								else if (format[ctrl_end] == '#') {
									pound_count++;
									if (pound_count > 1) {
										Result(0, "warning") = "Found multiple '#' after control '%' character. End fprintf early.";
										Result(0, "ftell_after") = ftell(_pFile);
										return;
									}
								}
							}

							// get sub-string specifying control type
							std::string thisCtrl = format.substr(n, ctrl_end);

							//if not more args, return
							//if this argument is empty, skip
							//if %s type, printout entire argument, otherwise just print single element
							if (args.size() <= current_arg) {//no more args, return
								Result(0, "warning") = "Additional intems in format string but not enough arguments. End fprintf early.";
								Result(0, "ftell_after") = ftell(_pFile);
								return;
							}
							else if (thisCtrl.back() == 's') {

							}
							else {

							}

						}
					}
				}




				break;
			}
			default:
				throw(std::runtime_error(std::string("Unknown function: ") + function_name));
			}
		}

	public:
		/** Close file and destroy object
		*/
		virtual ~AsyncFile() {
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


	};


}}