/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/

#pragma once

#define NOMINMAX
//#include <extras/stacktrace_error.hpp>
#include <cmath>
#include <algorithm>
#include <type_traits>

namespace extras {namespace ParticleTracking {

	/** template wrapper for radialavg<> accepting c-style numeric array as image data
	* returns tuple with
	* get<0>(out) -> average at each radial bin
	* get<1>(out) -> radial bin locations (if computeRloc==true)
	* get<2>(out) -> counts in each radial bin
	* Inputs:
	*   const M* img, size_t nRows, size_t nCols, //input image and size
	*	double x0, double y0, //location around which radial average is computed (0,0 is top left of image)
	*	double* imavg, size_t nAvg, //output array and number of elements
	*	double Rmax, //max radius to average over
	*	double Rmin, // min radius to average over
	*	double BinWidth = 1,//optinal bin width
	*	double * rLoc = nullptr, //optional output array specifying radii coordinates of bins in imavg. Must be same size as imavg
	*	CountsType * Counts = nullptr //optional output array with counts in each bin. Must be same size as imavg
	*/
	template<typename M,typename CountsType = size_t>
	void radialavg(
		const M* img, size_t nRows, size_t nCols, //input image and size
		double x0, double y0, //location around which radial average is computed (0,0 is top left of image)
		double* imavg, size_t nAvg, //output array and number of elements
		double Rmax, //max radius to average over
		double Rmin, // min radius to average over
		double BinWidth = 1,//optinal bin width
		double * rLoc = nullptr, //optional output array specifying radii coordinates of bins in imavg. Must be same size as imavg
		CountsType * Counts = nullptr //optional output array with counts in each bin. Must be same size as imavg
		)
	{
		using namespace std;

		bool free_counts = (Counts == nullptr);
		if (free_counts) {
			Counts = (CountsType*)std::malloc(nAvg * sizeof(CountsType));
		}

		try {
			//validate bin count
			size_t nBins = floor((Rmax - Rmin) / BinWidth) + 1;
			if (nBins > nAvg) {
				throw("radialavg(): nAvg<nBins, output may not be properly sized");
			}

			//zero-out imavg
			for (size_t n = 0; n < nAvg; n++) {
				imavg[n] = 0;
				Counts[n] = 0;
			}

			double BinWidth_2 = BinWidth / 2; //BinWidth/2
			double Rlim = Rmax + BinWidth_2; //Rmax+BinWidth/2
			double Rlim2 = pow(Rlim, 2);
			double Rinner = fmax(0, Rmin - BinWidth_2);
			double Rinner2 = pow(Rinner, 2);

			if (rLoc != nullptr) {
				for (size_t n = 0; n<nBins; ++n) {
					rLoc[n] = Rmin + BinWidth * n;
				}
				for (size_t n = nBins; n < nAvg; n++) {
					rLoc[n] = NAN;
				}
			}

			for (size_t xi = std::max(int(0), int(floor(x0 - Rlim))); xi <= min(int(nCols - 1), int(ceil(x0 + Rlim))); ++xi) {
				if (xi >= (x0 - Rlim) && xi <= (x0 + Rlim)) {
					double x2 = pow(xi - x0, 2);

					double yedge = sqrt(Rlim2 - x2);

					if (x2 >= Rinner2) { //left/right outer edges of donut
						for (size_t yi = max(int(0), int(floor(y0 - yedge)));
							yi <= min(int(nRows - 1), int(y0 + yedge)); ++yi)
						{
							if (is_integral<M>::value || !isnan(img[yi + nRows * xi])) { //if value is nan, just skip, otherwise compute bin location and add to sum
																			  //determine bin id
								size_t id = ceil((sqrt(pow(xi - x0, 2) + pow(yi - y0, 2)) - Rmin) / BinWidth - 0.5);
								if (id<nBins) {
									imavg[id] += img[yi + nRows * xi];//I(yi, xi);
									Counts[id]++;
								}
							}
						}
					}
					else { //inner hole region
						double yinner = sqrt(Rinner2 - x2);
						//lower half
						for (int yi = max(int(0), int(floor(y0 - yedge)));
							yi <= min(int(nRows - 1), int(y0 - yinner)); ++yi) {
							if (is_integral<M>::value || !isnan(img[yi + nRows * xi])) { //if value is nan, just skip, otherwise compute bin location and add to sum
																			  //determine bin id
								size_t id = ceil((sqrt(pow(xi - x0, 2) + pow(yi - y0, 2)) - Rmin) / BinWidth - 0.5);
								if (id<nBins) {
									imavg[id] += img[yi + nRows * xi]; //I(yi, xi);
									Counts[id]++;
								}
							}
						}
						//upper half
						for (int yi = max(int(0), int(floor(y0 + yinner)));
							yi <= min(int(nRows - 1), int(y0 + yedge)); ++yi) {
							if (is_integral<M>::value || !isnan(img[yi + nRows * xi])) { //if value is nan, just skip, otherwise compute bin location and add to sum
																			  //determine bin id
								size_t id = ceil((sqrt(pow(xi - x0, 2) + pow(yi - y0, 2)) - Rmin) / BinWidth - 0.5);
								if (id<nBins) {
									imavg[id] += img[yi + nRows * xi]; //I(yi, xi);
									Counts[id]++;
								}
							}
						}
					}

				}
			}

			//compute averages
			for (size_t n = 0; n<nBins; ++n) {
				if (Counts[n] == 0) {
					imavg[n] = NAN;
				}
				else {
					imavg[n] /= Counts[n];
				}
			}

			// if R==0 was not calculated because of rounding
			// set Ir[r=0] = I(round(y0),round(x0))
			if (Rmin == 0 && isnan(imavg[0])) {
				int rx0 = (int)round(x0);
				int ry0 = (int)round(y0);
				if (rx0 >= 0 && rx0 < nCols && ry0 >= 0 && ry0 < nRows) {
					imavg[0] = img[ry0 + nRows * rx0]; //I(ry0, rx0);
				}
			}
		}
		catch (...) {
			if (free_counts) {
				std::free(Counts);
			}
			throw;
		}
		
		// free memory
		if (free_counts) {
			std::free(Counts);
		}

	}

}}