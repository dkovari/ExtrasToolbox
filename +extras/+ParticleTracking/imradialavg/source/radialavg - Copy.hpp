/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once
//#include <mex.h>

#include <extras/Array.hpp> //include extras::extras::Array, be sure to add +extras/include to your include path

#include <cmath>
#include <tuple>

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

	    double BinWidth_2 = BinWidth/2; //BinWidth/2
	    double Rlim = Rmax+BinWidth_2; //Rmax+BinWidth/2
		double Rlim2 = pow(Rlim, 2);
		double Rinner = fmax(0, Rmin - BinWidth_2);
		double Rinner2 = pow(Rinner, 2);

		if (computeRloc) {
			RadiusPoints.resize_nocpy(nBins, 1);
			for (size_t n = 0; n<nBins; ++n) {
				RadiusPoints[n] = Rmin + BinWidth*n;
			}
		}

	    for(size_t xi=max(int(0),int(floor(x0-Rlim)));xi<=min(int(I.nCols()-1),int(ceil(x0+Rlim)));++xi){
	        if(xi>=(x0-Rlim) && xi<=(x0+Rlim) ){
	            double x2 = pow(xi-x0,2);

	            double yedge = sqrt(Rlim2 - x2);

				if (x2 >= Rinner2) { //outer edges of donut
					for (size_t yi = max(int(0), int(floor(y0 - yedge)));
						yi <= min(int(I.nRows() - 1), int(y0 + yedge)); ++yi)
					{
						if (is_integral<M>::value || !isnan(I(yi, xi))) { //if value is nan, just skip, otherwise compute bin location and add to sum
																		  //determine bin id
							size_t id = ceil((sqrt(pow(xi - x0, 2) + pow(yi - y0, 2)) - Rmin) / BinWidth - 0.5);
							if (id<nBins) {
								results[id] += I(yi, xi);
								Counts[id]++;
							}
						}
					}
				}
				else { //inner hole region
					double yinner = sqrt(Rinner2 - x2);
					//lower half
					for (int yi = max(int(0), int(floor(y0 - yedge)));
						yi <= min(int(I.nRows() - 1), int(y0 - yinner)); ++yi) {
						if (is_integral<M>::value || !isnan(I(yi, xi))) { //if value is nan, just skip, otherwise compute bin location and add to sum
																		  //determine bin id
							size_t id = ceil((sqrt(pow(xi - x0, 2) + pow(yi - y0, 2)) - Rmin) / BinWidth - 0.5);
							if (id<nBins) {
								results[id] += I(yi, xi);
								Counts[id]++;
							}
						}
					}
					//upper half
					for (int yi = max(int(0), int(floor(y0 + yinner)));
						yi <= min(int(I.nRows() - 1), int(y0 + yedge)); ++yi) {
						if (is_integral<M>::value || !isnan(I(yi, xi))) { //if value is nan, just skip, otherwise compute bin location and add to sum
																		  //determine bin id
							size_t id = ceil((sqrt(pow(xi - x0, 2) + pow(yi - y0, 2)) - Rmin) / BinWidth - 0.5);
							if (id<nBins) {
								results[id] += I(yi, xi);
								Counts[id]++;
							}
						}
					}
				}

	        }
	    }

	    //compute averages
	    for(size_t n=0;n<nBins;++n){
	        if (Counts[n]==0){
	            results[n]=NAN;
	        }
	        else{
	            results[n]/=Counts[n];
	        }
	    }

		// if R==0 was not calculated because of rounding
		// set Ir[r=0] = I(round(y0),round(x0))
		if (Rmin == 0 && isnan(results[0]) ) {
			double rx0 = round(x0);
			double ry0 = round(y0);
			if (rx0 >= 0 && rx0 < I.nCols() && ry0 >= 0 && ry0 < I.nRows()) {
				results[0] = I(ry0,rx0);
			}
		}

		return out;

	}

}}
