/*----------------------------------------------------------------------
Copyright -2019, Daniel T. Kovari, Emory University
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

// Includes for stack tracing

#ifdef _WIN32
#include <windows.h>
#include <StackWalker/StackWalker.h>
#else //mac & linux
#include <execinfo.h>
#include <cxxabi.h>
#endif

#include <stdexcept>

#ifdef _WIN32
namespace extras_stackwalker {
	class MyStackWalker : public StackWalker
	{
	public:
		MyStackWalker() : StackWalker(), _stack_msg(""){}
		std::string getStackMessage() { return _stack_msg; }
	protected:
		std::string _stack_msg;
		virtual void OnOutput(LPCSTR szText) {
			_stack_msg += szText;
		}
		virtual void OnLoadModule(LPCSTR    img,
			LPCSTR    mod,
			DWORD64   baseAddr,
			DWORD     size,
			DWORD     result,
			LPCSTR    symType,
			LPCSTR    pdbName,
			ULONGLONG fileVersion) {};

		virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) {};

		virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) {};
	};
};
#endif


namespace extras {

	class stacktrace_error : public std::exception {
		std::string _what;
	public:
		stacktrace_error(const std::string& what_str = "") {

			_what = "Exception Thrown: " + what_str;
			_what += "\n";
			_what += "====================================================\n";

#ifdef _WIN32
			CONTEXT c;
			GET_CURRENT_CONTEXT_STACKWALKER_CODEPLEX(c, CONTEXT_FULL);
			extras_stackwalker::MyStackWalker sw;
			sw.ShowCallstack(GetCurrentThread(), &c);
			_what += "\n";
			_what += sw.getStackMessage();
#else
			//  record stack trace upto 128 frames
			int callstack[128] = {};

			// collect stack frames
			int  frames = backtrace((void**)callstack, 128);

			// get the human-readable symbols (mangled)
			char** strs = backtrace_symbols((void**)callstack, frames);

			for (int i = 0; i < frames; ++i)
			{
				char functionSymbol[1024] = {};
				char moduleName[1024] = {};
				int  offset = 0;
				char addr[48] = {};

				/*

				Typically this is how the backtrace looks like:

				0   <app/lib-name>     0x0000000100000e98 _Z5tracev + 72
				1   <app/lib-name>     0x00000001000015c1 _ZNK7functorclEv + 17
				2   <app/lib-name>     0x0000000100000f71 _Z3fn0v + 17
				3   <app/lib-name>     0x0000000100000f89 _Z3fn1v + 9
				4   <app/lib-name>     0x0000000100000f99 _Z3fn2v + 9
				5   <app/lib-name>     0x0000000100000fa9 _Z3fn3v + 9
				6   <app/lib-name>     0x0000000100000fb9 _Z3fn4v + 9
				7   <app/lib-name>     0x0000000100000fc9 _Z3fn5v + 9
				8   <app/lib-name>     0x0000000100000fd9 _Z3fn6v + 9
				9   <app/lib-name>     0x0000000100001018 main + 56
				10  libdyld.dylib      0x00007fff91b647e1 start + 0

				*/

				// split the string, take out chunks out of stack trace
				// we are primarily interested in module, function and address
				sscanf(strs[i], "%*s %s %s %s %*s %d",
					&moduleName, &addr, &functionSymbol, &offset);

				int   validCppName = 0;
				//  if this is a C++ library, symbol will be demangled
				//  on success function returns 0
				//
				char* functionName = abi::__cxa_demangle(functionSymbol,
					NULL, 0, &validCppName);

				char stackFrame[4096] = {};
				if (validCppName == 0) // success
				{
					sprintf(stackFrame, "(%s)\t0x%s — %s + %d",
						moduleName, addr, functionName, offset);
				}
				else
				{
					//  in the above traceback (in comments) last entry is not
					//  from C++ binary, last frame, libdyld.dylib, is printed
					//  from here
					sprintf(stackFrame, "(%s)\t0x%s — %s + %d",
						moduleName, addr, functionName, offset);
				}

				if (functionName)
				{
					free(functionName);
				}

				std::string frameStr(stackFrame);
				
				_what += "\n";
				_what += frameStr;
			}
			free(strs);
#endif
		}

		virtual const char* what() const throw() {
			return _what.c_str();
		}
	};

};
