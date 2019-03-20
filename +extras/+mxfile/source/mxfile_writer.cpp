#include <extras/mxfile/mxfile_writer.hpp>
#include <mex.h>
#include <string>
#include <extras/cmex/mexextras.hpp>
/*TO DO: 
	-Write a class that interfaces filewriter to Matlab with async library (refer to mexInterfaceTest and ExampleProcessor
	-Write an object manager that creates filewriters from a pointer 
	-Fill in the mex function with the actual thing we want to do
	-What kind of behavior do we want from the MATLAB end? extras.filewriter.write(____) and fileReader.read(___)?

*/




void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	/*
		function write_test(filepath, varargin)
		Required Input
		filepath: mxChar array specifying path to write to
		varargin: variables to write to the file
		*/

	// validate input
	if (nrhs < 1) {
		throw(std::runtime_error("Requires at least one argument"));
	}

	std::string filepath = extras::cmex::getstring(prhs[0]);

	mexPrintf("Will use %s for writing\n", filepath.c_str());

	// serialize the data
	mexPrintf("About to serialize data\n");
	auto sD = extras::mxfile::Serialize(nrhs - 1, &(prhs[1])); //skip first argument;

	// write to file
	mexPrintf("Writing serialized data\n");
	extras::mxfile::writeList(sD, filepath.c_str());


	return;
}