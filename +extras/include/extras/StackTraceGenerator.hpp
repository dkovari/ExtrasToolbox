/* StackTraceGenerator.hpp
Code for generating human-readable stack traces from within a program

Originally found at: https://www.nullptr.me/2013/04/14/generating-stack-trace-on-os-x/
(Downloaded on 2019-05-09)
Appears to be written by Sarang Baheti is 2018

Original Blog Comments:
Stack trace is every developer’s tool when it comes to looking at the path of execution. While investigating assertion or error, I typically prefer looking at stack trace first and then at the code. Back tracing is specific to each OS. Windows defines several functions, I will cover that in another post, while Mac OS X uses BSD style back_trace functions.

It’s fairly easy to code up a class that generates the stack trace on Mac OS X, but has a caveat that line numbers and file-names are not yet there. This is something for me to figure out.

*/


#pragma once


#include <iostream>
#include <execinfo.h>
#include <cxxabi.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <memory>

/** Class for generating vector of strings listing the functions in the thread's callstack
* Usage:
*	std::vector<std::string> thisStack = StackTraceGenerator::GetTrace();
*/
class StackTraceGenerator
{
private:

	//  this is a pure utils class
	//  cannot be instantiated
	//
	StackTraceGenerator() = delete;
	StackTraceGenerator(const StackTraceGenerator&) = delete;
	StackTraceGenerator& operator=(const StackTraceGenerator&) = delete;
	~StackTraceGenerator() = delete;

public:

	static std::vector<std::string> GetTrace()
	{
		//  record stack trace upto 128 frames
		int callstack[128] = {};

		// collect stack frames
		int  frames = backtrace((void**)callstack, 128);

		// get the human-readable symbols (mangled)
		char** strs = backtrace_symbols((void**)callstack, frames);

		std::vector<std::string> stackFrames;
		stackFrames.reserve(frames);

		for(int i = 0; i < frames; ++i)
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
			stackFrames.push_back(frameStr);
		}
		free(strs);

		return stackFrames;
	}
};