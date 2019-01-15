/*
[X,Y] = BaryCenter_classic(Image, WIND, Sz, LimFrac)
Input:
Image: 2D matrix with image data
WIND: [n x 4] array specifying subwindows to process
if not specified, defaults to entire image
Sz: not used, set to anything
LimFrac: Threshold factor for selecting brightest and darkest regions
default=0.2
Threshold is calculated as:
Low = Range*LimFrac + min
UP = Range*(1-LimFrac) + min
Outputs:
[X,Y] coordinates for each window
if something went wrong returns NaN
*/


#define DEFAULT_LIM_FRAC 0.2

#include <mex.h>
#include <cmath>
#include "MatrixT.hpp"
#include <string>
#include "SubImage.h"
#include "SimpleMatrix.h"


#include "barycenter_classic.h"

template<typename T>
inline T vmax(T a, T b) {
	return (a >= b) ? a : b;
}

template<typename T>
inline T vmin(T a, T b) {
	return (a <= b) ? a : b;
	//return a; //this is a mistake
}


using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	//validate Inputs
	if (nrhs<1) {
		mexErrMsgTxt("Not enough inputs");
	}

	if (mxIsComplex(prhs[0])) {
		throw("Input Image must not be complex.");
	}

	if (mxGetNumberOfDimensions(prhs[0]) != 2) {
		throw("Input Image must be a matrix, i.e. ndim(Image)==2");
	}

	size_t HEIGHT = mxGetM(prhs[0]);
	size_t WIDTH = mxGetN(prhs[0]);

	// SECOND INPUT: WIND
	mexMatrixT<double> WIND; //[x,y,W,H]
	if (nrhs>1) {
		WIND = prhs[1];
	}

	if (WIND.IsEmpty()) {
		WIND.resize(1, 4);
		WIND[0] = 0; //x
		WIND[1] = 0;//y
		WIND[2] = WIDTH; //W
		WIND[3] = HEIGHT; //H
	}

	if (WIND.nCols() != 4) {
		mexErrMsgTxt("WIND must be n x 4 matrix");
	}
#ifdef DEBUG_PLOT
	mexPrintf("WIND:\n");
	WIND.disp();
#endif

	// THIRD INPUT: size limits --> ignored


	// FOURTH INPUT: LimFrac
	double LimFrac;
	if (nrhs<4) {// no LimFrac defined
		LimFrac = DEFAULT_LIM_FRAC;
	}
	else {
		LimFrac = mxGetScalar(prhs[3]);
	}
#ifdef DEBUG_PLOT
	mexPrintf("LimFrac:\n");
	mexPrintf("\t%g\n", LimFrac);
#endif

	mexMatrixT<double> X;
	mexMatrixT<double> Y;
	X.resize(WIND.nRows(), 1);
	Y.resize(WIND.nRows(), 1);

	//loop over windows and find positions
	for (size_t w = 0; w<WIND.nRows(); w++) {
		X[w] = mxGetNaN(); //default value is nan
		Y[w] = mxGetNaN(); //default value is nan


		size_t X0 = fmax(0, fmin(WIDTH - 1, WIND(w, 0)));
		size_t Y0 = fmax(0, fmin(HEIGHT - 1, WIND(w, 1)));
		size_t W = WIND(w, 2);
		size_t H = WIND(w, 3);
		size_t X1 = fmax(0, fmin(WIDTH - 1, X0 + W - 1));
		size_t Y1 = fmax(0, fmin(HEIGHT - 1, Y0 + H - 1));
		W = X1 - X0 + 1;
		H = Y1 - Y0 + 1;


		if (X0 >= X1 || Y0 >= Y1) { //zero width/height probably because window is out of bounds
			continue; //just process next window
		}

		// Get Sub Image using window bounds
		SubImage windImg = mx2SubImage(prhs[0], Y0, X0, H, W);

#ifdef DEBUG_PLOT
		SimpleMatrix<double> wI(H, W);
		for (size_t x = 0; x<W; ++x) {
			for (size_t y = 0; y<H; ++y) {
				wI(y, x) = windImg(y, x);
			}
		}
		mexEvalString("figure");
		imagesc(wI);
		mexEvalString("title('windImg');");
#endif

		double y;
		double x;

		switch (windImg.ClassID()) {
		case mxUINT8_CLASS:
			mass_center((uint8_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxINT8_CLASS:
			mass_center((int8_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxUINT16_CLASS:
			mass_center((uint16_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxINT16_CLASS:
			mass_center((int16_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxUINT32_CLASS:
			mass_center((uint32_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxINT32_CLASS:
			mass_center((int32_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxUINT64_CLASS:
			mass_center((uint64_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxINT64_CLASS:
			mass_center((int64_t*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		case mxDOUBLE_CLASS:
			mass_center((double*)windImg.data(), 0, H, W, windImg.stride(), LimFrac, 50, &y, &x);
			break;
		default:
			throw(std::runtime_error("Image class not implemented"));
		}
		

		X[w] = x+double(X0)+1;
		Y[w] = y+double(Y0)+1;

	}

	//set outputs

	plhs[0] = X;
	if (nlhs>1) {
		plhs[1] = Y;
	}

}
