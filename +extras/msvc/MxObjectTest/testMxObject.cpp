/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#include <extras/cmex/NumericArray.hpp>
#include <extras/Array.hpp>
#include <extras/cmex/MxCellArray.hpp>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	extras::cmex::NumericArray<double> out(10);


	out[5] = 5;
	out(3, 0) = 33;

	out.reshape({ 5,1 });

	out.disp();


	mexPrintf("Concatenate\n");
	extras::cmex::NumericArray<double> second(5,1);
	second[1] = 77;
	second[2] = 99;
	second.disp();
	out.concatenate(second, 2);

	mexPrintf("cat with array\n");
	extras::Array<double> A(5,1,true);
	A[2] = 222;
	A[3] = 666;

	A.disp();

	out.concatenate(A, 2);

	out.disp();

	plhs[0] = out;

	if (nrhs > 0) {
		extras::cmex::NumericArray<double> in(prhs[0]);
		in.disp();
		in.makePersistent();
		in.disp();
	}

	//////////////
	// Test CellArray

	extras::cmex::MxCellArray CA;
	CA.reshape(2, 1);

	plhs[1] = CA;
	

	mxArray* c = mxCreateCellMatrix(2, 1);
	mxSetCell(c, 0, mxCreateDoubleScalar(1));
	mxSetCell(c, 1, mxCreateDoubleScalar(2));

	mexPrintf("c: %p\n", c);
	mexPrintf("mxGetData(c): %p\n", mxGetData(c));
	mexPrintf("mxGetElementSize(c): %p\n", mxGetElementSize(c));
	mexPrintf("mxGetCell(c,0): %p\n", mxGetCell(c,0));
	mexPrintf("mxGetCell(c,1): %p\n", mxGetCell(c, 1));

	void* data = mxGetData(c);
	mxArray* p0 = ((mxArray**)data)[0];
	mxArray* p1 = ((mxArray**)data)[1];

	mexPrintf("p0: %p\n", p0);
	mexPrintf("p1: %p\n", p1);


	const char* field1 = "test";
	const char* field2 = "test2";
	const char* fn[] = { field1,field2 };

	mxArray* s = mxCreateStructMatrix(2, 1, 2, fn);
	mexPrintf("mxGetData(s): %p\n", mxGetData(s));
	mexPrintf("mxGetElementSize(s): %p\n", mxGetElementSize(s));

	mexPrintf("field0: %s\n",mxGetFieldNameByNumber(s, 0));
	mexPrintf("field1: %s\n", mxGetFieldNameByNumber(s, 1));

	mxSetFieldByNumber(s, 0, 0, mxCreateDoubleScalar(0));
	mxSetFieldByNumber(s, 1, 0, mxCreateDoubleScalar(1));
	mxSetFieldByNumber(s, 0, 1, mxCreateDoubleScalar(2));
	mxSetFieldByNumber(s, 1, 1, mxCreateDoubleScalar(3));

	mexPrintf("mxGetFieldByNumber(s, 0, 0): %p\n", mxGetFieldByNumber(s, 0, 0));
	mexPrintf("mxGetFieldByNumber(s, 1, 0): %p\n", mxGetFieldByNumber(s, 1, 0));
	mexPrintf("mxGetFieldByNumber(s, 0, 1): %p\n", mxGetFieldByNumber(s, 0, 1));
	mexPrintf("mxGetFieldByNumber(s, 1, 1): %p\n", mxGetFieldByNumber(s, 1, 1));

	void* dataS = mxGetData(s);
	mxArray* Sp0 = ((mxArray**)dataS)[0];
	mxArray* Sp1 = ((mxArray**)dataS)[1];

	mexPrintf("Sp0: %p\n", Sp0);
	mexPrintf("Sp1: %p\n", Sp1);

}