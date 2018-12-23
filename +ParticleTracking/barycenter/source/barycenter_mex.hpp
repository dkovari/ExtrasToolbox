#pragma once

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

#include <extras/cmex/NumericArray.hpp>
#include "barycenter.hpp"

namespace extras{namespace ParticleTracking{

	void barycenter_mex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
	{
		using namespace std;
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

		cmex::NumericArray<double> WIND;//init empty window
		if(nrhs>1){
			WIND = prhs[1];
		}

		double LimFrac = 0.2;
		if(nrhs>3){
			LimFrac = mxGetScalar(prhs[3]);
		}

		std::vector<cmex::NumericArray<double>> out;

		switch(mxGetClassID(prhs[0])){
			case mxUINT8_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,uint8_t>(cmex::NumericArray<uint8_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxINT8_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,int8_t>(cmex::NumericArray<int8_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxUINT16_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,uint16_t>(cmex::NumericArray<uint16_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxINT16_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,int16_t>(cmex::NumericArray<int16_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxUINT32_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,uint32_t>(cmex::NumericArray<uint32_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxINT32_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,int32_t>(cmex::NumericArray<int32_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxUINT64_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,uint64_t>(cmex::NumericArray<uint64_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxINT64_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,int64_t>(cmex::NumericArray<int64_t>(prhs[0]),WIND, LimFrac);
				break;
			case mxDOUBLE_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,double>(cmex::NumericArray<double>(prhs[0]),WIND, LimFrac);
				break;
			case mxSINGLE_CLASS:
			 	out = barycenter<cmex::NumericArray<double>,float>(cmex::NumericArray<float>(prhs[0]),WIND, LimFrac);
				break;
			default:
				throw(std::runtime_error("Image was unsupported type."));
				break;
		}

		//set outputs
		plhs[0] = out[0];
		if (nlhs>1) {
			plhs[1] = out[1];
		}

	}
}}
