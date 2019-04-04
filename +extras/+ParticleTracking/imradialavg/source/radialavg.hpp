/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once
//#include <mex.h>

#include <extras/Array.hpp> //include extras::extras::Array, be sure to add +extras/include to your include path
#include <tuple>

#include "radialavg.h"

namespace extras{namespace ParticleTracking{
	/// Compute Azmuthal average of an image around a fixed point
	/// Assume column-major data with zero indexing.
	///Inputs:
	///  I: Image data
	///  x0: x position of center point
	///  y0: y position of center point
	///  Rmax=NAN: maximum radius to include in the average (if NAN, uses largest radius available for the given image and x0,y0 coordinate)
	///  BinWidth=1: width of the bins (in pixels)
	///  computeRloc=true: specify if output should include R locations
	///Output:
	/// Tuple with 3 elements
	/// get<0>(out) -> average at each radial bin
	/// get<1>(out) -> radial bin locations (if computeRloc==true)
	/// get<2>(out) -> counts in each radial bin
	///
	/// Output types are determined by the template arguments
	template<typename resultsT=double, class resultsArrayClass=extras::Array<double>,
			typename locT=size_t, class locArrayClass=extras::Array<locT>,
			typename countT=size_t, class countArrayClass=extras::Array<countT>,
	        typename M=double>
	std::tuple<resultsArrayClass,locArrayClass, countArrayClass>
	radialavg(const extras::ArrayBase<M>& I, double x0, double y0, double Rmax = NAN, double Rmin = 0, double BinWidth = 1,bool computeRloc = true){

		using namespace std;

		//mexPrintf("In main radial avg: I:%d\n", &I);

		std::tuple<resultsArrayClass, locArrayClass, countArrayClass> out;
		resultsArrayClass& results = std::get<0>(out);
		locArrayClass& RadiusPoints = std::get<1>(out);
		countArrayClass& Counts = std::get<2>(out);


	    if(!isfinite(Rmax)){
	        Rmax =
	        fmax( sqrt(x0*x0+y0*y0),
	            fmax(sqrt( pow(I.nCols()-1-x0,2) + y0*y0),
	                fmax(sqrt(pow(I.nCols()-1-x0,2) + pow(I.nRows()-1-y0,2)),
	                    sqrt(x0*x0 + pow(I.nRows()-1-y0,2)))));
	    }else{
	        if(Rmin>Rmax){
	            swap(Rmin,Rmax);
	        }
	    }
	    Rmax = fmax(0,Rmax);
	    Rmin = fmax(0,Rmin);

	    size_t nBins = floor((Rmax-Rmin)/BinWidth)+1;
		results.resize_clear(nBins, 1);
		Counts.resize_clear(nBins, 1);
		results.getdata();


		if (computeRloc) {
			RadiusPoints.resize_nocpy(nBins, 1);
			radialavg(I.getdata(), I.nRows(), I.nCols(), //input image and size
				x0,y0,
				results.getdata(), nBins, //output array and number of elements
				Rmax, //max radius to average over
				Rmin, // min radius to average over
				BinWidth,//optinal bin width
				RadiusPoints.getdata(), //optional output array specifying radii coordinates of bins in imavg. Must be same size as imavg
				Counts.getdata() //optional output array with counts in each bin. Must be same size as imavg
			);
			//radialavg(I.getdata(), I.nRows(), I.nCols(), results.getdata(), results.numel(), Rmax, Rmin, BinWidth, RadiusPoints.getdata(), Counts.getdata());
		}
		else {
			radialavg(I.getdata(), I.nRows(), I.nCols(), //input image and size
				x0, y0,
				results.getdata(), nBins, //output array and number of elements
				Rmax, //max radius to average over
				Rmin, // min radius to average over
				BinWidth,//optinal bin width
				nullptr, //optional output array specifying radii coordinates of bins in imavg. Must be same size as imavg
				Counts.getdata() //optional output array with counts in each bin. Must be same size as imavg
			);
		}

		return out;

	}

}}
