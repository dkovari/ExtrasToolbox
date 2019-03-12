/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#include <extras/cmex/NumericArray.hpp>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	extras::cmex::NumericArray<double> out(10);

	out[5] = 5;
	out(3, 0) = 33;

	out.disp();

	plhs[0] = out;

	if (nrhs > 0) {
		extras::cmex::NumericArray<double> in(prhs[0]);
		in.disp();
		in.makePersistent();
		in.disp();
	}
}