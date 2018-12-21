#include <extras/async/AsyncProcessor.hpp>
#include <extras/mxfile/FileWriter.hpp>
#include <mex.h>
/*TO DO: 
	-Write a class that interfaces filewriter to Matlab with async library (refer to mexInterfaceTest and ExampleProcessor
	-Write an object manager that creates filewriters from a pointer 
	-Fill in the mex function with the actual thing we want to do
	-What kind of behavior do we want from the MATLAB end? extras.filewriter.write(____)?
*/



void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	mexPrintf("Hi James!\n");
//	mxArray* filepath;
//	filepath = mxCreateString("Enter a filepath: \n");
//	mexCallMATLAB(&filepath, );
//	extras::mxfile::writeList(extras::mxfile::Serialize(nrhs, prhs), filepath);
}