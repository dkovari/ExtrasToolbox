#include <mex.h>
#include <mexNumericArray.hpp>
#include "radialavg.hpp"

// template wrapper for radialavg<> so that mxArray is cast to the corresponding NumericArray<> type
std::tuple<mex::NumericArray<double>, mex::NumericArray<double>, mex::NumericArray<double>>
radialavg(const mxArray* mxI, double x, double y, double Rmax, double Rmin, double BinWidth)
{

	switch (mxGetClassID(mxI)) { //handle different image types seperatelys
	case mxDOUBLE_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<double>(mxI), x, y,Rmax, Rmin, BinWidth);
	case mxSINGLE_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<float>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxINT8_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<int8_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxUINT8_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<uint8_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxINT16_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<int16_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxUINT16_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<uint16_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxINT32_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<int32_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxUINT32_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<uint32_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxINT64_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<int64_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	case mxUINT64_CLASS:
		return radialavg<double, mex::NumericArray<double>, double, mex::NumericArray<double>, double, mex::NumericArray<double>>
			(mex::NumericArray<uint64_t>(mxI), x, y, Rmax, Rmin, BinWidth);
	default:
		throw(std::runtime_error("radialavg: Only numeric image types allowed"));
	}
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if(nrhs<3){
        mexErrMsgTxt("Three inputs required");
    }

	if (mxGetNumberOfElements(prhs[1]) != mxGetNumberOfElements(prhs[2])) {
		mexErrMsgTxt("numel x must be same as numel y");
	}

	mex::NumericArray<double> X(prhs[1]);
	mex::NumericArray<double> Y(prhs[2]);

	if (X.numel() > 1) {
		plhs[0] = mxCreateCellMatrix(X.numel(), 1);
	}

	mex::NumericArray<double> Rmax(1, 1);
	Rmax[0] = NAN;
    if(nrhs>3){
		Rmax = prhs[3];
    }

	if (Rmax.numel() != 1 && Rmax.numel() != X.numel()) {
		mexErrMsgTxt("Invalid number of Rmax");
	}

	mex::NumericArray<double> Rmin(1, 1);
    Rmin[0] = 0;
    if(nrhs>4){
		Rmin = prhs[4];
    }

	if (Rmin.numel() != 1 && Rmin.numel() != X.numel()) {
		mexErrMsgTxt("Invalid number of Rmin");
	}

	mex::NumericArray<double> BinWidth(1, 1);
    BinWidth[0] = 1;
    if(nrhs>5){
		BinWidth = prhs[5];
    }

	if (BinWidth.numel() != 1 && BinWidth.numel() != X.numel()) {
		mexErrMsgTxt("Invalid number of BinWidth");
	}

	bool NeedRadPts = nlhs > 1;
	bool NeedBinCounts = nlhs > 2;

	if (X.numel() > 1 && NeedRadPts) {
		plhs[1] = mxCreateCellMatrix(X.numel(), 1);
	}

	if (X.numel() > 1 && NeedBinCounts) {
		plhs[2] = mxCreateCellMatrix(X.numel(), 1);
	}


	for (size_t n = 0; n < X.numel(); ++n) {

		// Calc Rad avg
		if (X.numel() == 1) {
			auto res = radialavg(prhs[0], X[n] - 1, Y[n] - 1, Rmax[0], Rmin[0], BinWidth[0]);

			plhs[0] = std::get<0>(res);

			if (nlhs > 1)
			{
				plhs[1] = std::get<1>(res);
			}

			if (nlhs > 2)
			{
				plhs[2] = std::get<2>(res);
			}

		}
		else {//cell array outputs

			double Rmx = Rmax[0];
			if (Rmax.numel() > 1) {
				Rmx = Rmax[n];
			}
			double Rmn = Rmin[0];
			if (Rmin.numel() > 1) {
				Rmn = Rmin[n];
			}
			double Bw = BinWidth[0];
			if (BinWidth.numel() > 1) {
				Bw = BinWidth[n];
			}

			auto res = radialavg(prhs[0], X[n] - 1, Y[n] - 1, Rmx, Rmn, Bw);

			mxSetCell(plhs[0], n, std::get<0>(res));

			if (NeedRadPts) {
				mxSetCell(plhs[1], n, std::get<1>(res));
			}
			if (NeedBinCounts) {
				mxSetCell(plhs[2], n, std::get<2>(res));
			}
		}

	}

}
