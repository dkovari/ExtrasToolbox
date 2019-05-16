/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <extras/cmex/NumericArray.hpp>
#include "radialavg.hpp"

namespace extras{namespace ParticleTracking{

    /** template wrapper for radialavg<> so that mxArray is cast to the corresponding NumericArray<> type
	* returns tuple with
	* get<0>(out) -> average at each radial bin
	* get<1>(out) -> radial bin locations (if computeRloc==true)
	* get<2>(out) -> counts in each radial bin
	* Inputs:
	*	double x0, double y0, //location around which radial average is computed (0,0 is top left of image)
	*	double* imavg, size_t nAvg, //output array and number of elements
	*	double Rmax, //max radius to average over
	*	double Rmin, // min radius to average over
	*	double BinWidth = 1,//optinal bin width
	*	double * rLoc = nullptr, //optional output array specifying radii coordinates of bins in imavg. Must be same size as imavg
	*	CountsType * Counts = nullptr //optional output array with counts in each bin. Must be same size as imavg
	*/
    std::tuple<extras::cmex::NumericArray<double>, extras::cmex::NumericArray<double>, extras::cmex::NumericArray<double>>
    radialavg(const mxArray* mxI, //image
		double x, double y, //location around which radial average is computed (0,0 is top left of image)
		double Rmax, //max radius to average over
		double Rmin, // min radius to average over
		double BinWidth, //bin width
		bool computeRloc = true //flag specifying if r locations should be computed and stored in the output
		)
    {

    	switch (mxGetClassID(mxI)) { //handle different image types seperatelys
    	case mxDOUBLE_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<double>(mxI), x, y,Rmax, Rmin, BinWidth, computeRloc);
    	case mxSINGLE_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<float>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxINT8_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<int8_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxUINT8_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<uint8_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxINT16_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<int16_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxUINT16_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<uint16_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxINT32_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<int32_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxUINT32_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<uint32_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxINT64_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<int64_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	case mxUINT64_CLASS:
    		return radialavg<double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>, double, extras::cmex::NumericArray<double>>
    			(extras::cmex::NumericArray<uint64_t>(mxI), x, y, Rmax, Rmin, BinWidth, computeRloc);
    	default:
    		throw(extras::stacktrace_error("radialavg: Only numeric image types allowed"));
    	}
    }

	/** Callable MEX function
	* [Avg,BinLocations,BinCounts] = imradialavg(I,x0,y0,Rmax,Rmin,BinWidth)
	* Computer azmuthal average of image around specified location
	*Inputs:
	*   I: the image to use (should not be complex, but any other numeric type
	*       is fine)
	*   x0,y0: scalar numbers specifying the coordinates
	*           (NOTE: <1,1> is top left corner of image)
	*   Rmax(=NaN): scalar specifying maximum radius (NaN indicated image edges
	*       are the limits)
	*   Rmin(=0): minimum radius to use
	*       NOTE: If you specify both Rmax and Rmin you can use the more
	*       logical ordering: imradialavg(__,Rmin,Rmax);
	*   BinWidth(=1): width and spacing of the bins
	*
	* Outputs:
	*   Avg: radial averages
	*   BinLocations: locations of the radial bins (e.g. 0,1,...,Rmax)
	*   BinCounts: number of pixels accumulated into each bin
	*/
    void imradialavg_mex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
        if(nrhs<3){
            mexErrMsgTxt("Three inputs required");
        }

    	if (mxGetNumberOfElements(prhs[1]) != mxGetNumberOfElements(prhs[2])) {
    		mexErrMsgTxt("numel x must be same as numel y");
    	}

    	extras::cmex::NumericArray<double> X(prhs[1]);
    	extras::cmex::NumericArray<double> Y(prhs[2]);

    	if (X.numel() > 1) {
    		plhs[0] = mxCreateCellMatrix(X.numel(), 1);
    	}

    	extras::cmex::NumericArray<double> Rmax(1, 1);
    	Rmax[0] = NAN;
        if(nrhs>3){
    		Rmax = prhs[3];
        }

    	if (Rmax.numel() != 1 && Rmax.numel() != X.numel()) {
    		mexErrMsgTxt("Invalid number of Rmax");
    	}

    	extras::cmex::NumericArray<double> Rmin(1, 1);
        Rmin[0] = 0;
        if(nrhs>4){
    		Rmin = prhs[4];
        }

    	if (Rmin.numel() != 1 && Rmin.numel() != X.numel()) {
    		mexErrMsgTxt("Invalid number of Rmin");
    	}

    	extras::cmex::NumericArray<double> BinWidth(1, 1);
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
}}
