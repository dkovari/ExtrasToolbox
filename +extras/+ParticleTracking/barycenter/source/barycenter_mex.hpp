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

	template<class OutContainerClass> //C must be and ArrayBase derived class
    std::vector<OutContainerClass> barycenter_mx(const mxArray* pI,
                                const extras::ArrayBase<double>& WIND,
                                double LimFrac=0.2)
    {
        switch (mxGetClassID(pI)) { //handle different image types seperatelys
    	case mxDOUBLE_CLASS:
    		return barycenter<OutContainerClass,double>(cmex::NumericArray<double>(pI), WIND, LimFrac);
    	case mxSINGLE_CLASS:
    		return barycenter<OutContainerClass,float>(cmex::NumericArray<float>(pI), WIND, LimFrac);
    	case mxINT8_CLASS:
    		return barycenter<OutContainerClass,int8_t>(cmex::NumericArray<int8_t>(pI), WIND, LimFrac);
    	case mxUINT8_CLASS:
    		return barycenter<OutContainerClass,uint8_t>(cmex::NumericArray<uint8_t>(pI), WIND, LimFrac);
    	case mxINT16_CLASS:
    		return barycenter<OutContainerClass,int16_t>(cmex::NumericArray<int16_t>(pI), WIND, LimFrac);
    	case mxUINT16_CLASS:
    		return barycenter<OutContainerClass,uint16_t>(cmex::NumericArray<uint16_t>(pI), WIND, LimFrac);
    	case mxINT32_CLASS:
    		return barycenter<OutContainerClass,int32_t>(cmex::NumericArray<int32_t>(pI), WIND, LimFrac);
    	case mxUINT32_CLASS:
    		return barycenter<OutContainerClass,uint32_t>(cmex::NumericArray<uint32_t>(pI), WIND, LimFrac);
    	case mxINT64_CLASS:
    		return barycenter<OutContainerClass,int64_t>(cmex::NumericArray<int64_t>(pI), WIND, LimFrac);
    	case mxUINT64_CLASS:
    		return barycenter<OutContainerClass,uint64_t>(cmex::NumericArray<uint64_t>(pI), WIND, LimFrac);
    	default:
    		throw(std::runtime_error("radialcenter: Only numeric image types allowed"));
    	}
    }

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
			if(!WIND.isempty()){
                if(WIND.nCols()!=4){
                    throw(std::runtime_error("WIND must be [n x 4]"));
                }
                for(size_t n=0;n<WIND.nRows();++n){ //fix 1-index --> 0-index
                    WIND(n,0)-= 1;
                    WIND(n,1)-=1;
                }
            }
		}

		double LimFrac = 0.2;
		if(nrhs>3){
			LimFrac = mxGetScalar(prhs[3]);
		}

		auto out = barycenter_mx<cmex::NumericArray<double>>(prhs[0],WIND,LimFrac);

		out[0]+=1; //fix 1-indexing
		out[1]+=1;

		//set outputs
		plhs[0] = out[0];
		if (nlhs>1) {
			plhs[1] = out[1];
		}

	}
}}
