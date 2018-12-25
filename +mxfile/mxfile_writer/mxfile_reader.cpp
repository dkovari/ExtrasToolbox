#include <extras/mxfile/FileWriter.hpp>
#include <mex.h>
#include <string>
#include <extras/cmex/mexextras.hpp>
#include <list>



void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	/*
		function read_test(filepath)
		Required Input
		filepath: mxChar array specifying path to read from
		*/


		//mexPrintf("Hi James!\n");


		// validate input
	if (nrhs < 1) {
		throw(std::runtime_error("Requires at least one argument"));
	}

	std::string filepath = extras::cmex::getstring(prhs[0]);
	mexPrintf("Will use %s for reading\n", filepath.c_str());
	mexPrintf("Press a key to continue\n");
	mexEvalString("pause");

	FILE* fp = fopen(filepath.c_str(), "rb");

	std::list<mxArray*> out_list;
	while (!feof(fp)) {
		mexPrintf("Top of read loop fp=%i\n", ftell(fp));
		mexPrintf("reading next\n");
		mexPrintf("Press a key to continue\n");
		mexEvalString("pause");
		mxArray* res = extras::mxfile::readNext(fp);
		if (!res) {
			break;
		}
		out_list.push_back(res);
		mexPrintf("\t read type: %s\n", mxGetClassName(out_list.back()));
	}

	mexPrintf("Assembling output cell\n");
	mexPrintf("Press a key to continue\n");
	mexEvalString("pause");

	mxArray* out = mxCreateCellMatrix(out_list.size(), 1);
	size_t n = 0;
	for (mxArray* o : out_list) {
		mxSetCell(out, n, o);
		++n;
	}

	plhs[0] = out; //the first output arg is always valid even if nlhs==0
	fclose(fp);
	return;
}

