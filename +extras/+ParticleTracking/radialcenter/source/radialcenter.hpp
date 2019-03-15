/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <extras/Array.hpp> //include extras::extras::Array, be sure to add +extras/include to your include path

#include <math.h>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>

#include <mex.h>

#define DEFAULT_GP 5

namespace rcdefs {
	enum COM_METHOD { MEAN_ABS, NORMAL, GRAD_MAG };

	// Apply 3x3 average to image
	// Edges are corrected so they are 2x3 (corners are 2x2)
	// Inputs:
	//	double *I: pointer to image data
	// Ny: height
	// Nx: width
	// strideI: stride of I. I(y,x) = I[y+strideI*x]
	// O: pointer to pre-allocated Output
	// 	strideO: stride of output O(y,x) = O[y+strideO*x]
	// 	O must have same dim as I, but stride can be different
	template<typename M>
	void mean3x3(const M *I, size_t Ny, size_t Nx, size_t strideI, double* O, size_t strideO) {

		double * Ot = (double*)malloc(Ny*Nx * sizeof(double));
		// filter along x using 1x3
		for (size_t y = 0; y<Ny; ++y) {
			//handle first column separately
			Ot[y] = double(I[y] + I[y + strideI * 1]) / 2.0;

			//middle no special handling needed
			for (size_t x = 1; x<Nx - 1; ++x) {
				Ot[y + Ny*(x)] = double(I[y + strideI*(x - 1)] + I[y + strideI*(x)] + I[y + strideI*(x + 1)]) / 3.0;
			}

			//handle last column separately
			Ot[y + Ny*(Nx - 1)] = double(I[y + strideI*(Nx - 2)] + I[y + strideI*(Nx - 1)]) / 2.0;
		}

		//filter along y using 3x1
		for (size_t x = 0; x<Nx; ++x) {
			//handle first row separately
			O[strideO*x] = (Ot[Ny*x] + Ot[1 + Ny*x]) / 2.0;

			//middle no special handling needed
			for (size_t y = 1; y<Ny - 1; ++y) {
				O[y + strideO*(x)] = (Ot[y - 1 + Ny*(x)] + Ot[y + Ny*(x)] + Ot[y + 1 + Ny*(x)]) / 3.0;
			}

			//handle last row separately
			O[Ny - 1 + strideO*x] = (Ot[Ny - 2 + Ny*x] + Ot[Ny - 1 + Ny*x]) / 2.0;
		}

		free(Ot);

	}

	// Alias to mean3x3(...)
	// calls mean3x3 w/ strideI=stride)=Ny
	template <typename M>
	void mean3x3(const M *I, size_t Ny, size_t Nx, double* O) {
		mean3x3<M>(I, Ny, Nx, Ny, O, Ny);
	}

	// Calculate 3x3-smoothed image gradient
	// I is colum-major data pointing to I[y1+x1*stride]
	// du and dv should point to pre-allocated extras::Arrays of size dNy x dNx
	// corresponding to the window height and width:
	//		dNy = y2-y1; dNx = x2-x1;
	template<typename M>
	void smoothgrad(const M *I, size_t stride, double *du, double * dv, size_t dNy, size_t dNx) {

		//temp variables
		double *tmp_u, *tmp_v; //finite grad

		tmp_u = (double*)malloc(dNy*dNx * sizeof(double));
		tmp_v = (double*)malloc(dNy*dNx * sizeof(double));

		// calc finite difference
		for (size_t x = 0; x < dNx; ++x) {
			for (size_t y = 0; y < dNy; ++y) {
				tmp_u[y + x*dNy] = (double)I[(y + 1) + (x + 1)*stride] - (double)I[(y)+(x)*stride];
				tmp_v[y + x*dNy] = (double)I[(y + 1) + (x)*stride] - (double)I[(y)+(x + 1)*stride];
			}
		}

		// apply 3x3 smoothing
		mean3x3(tmp_u, dNy, dNx, du);
		mean3x3(tmp_v, dNy, dNx, dv);

		free(tmp_u);
		free(tmp_v);
	}

	// structure used for optional parameters of radial center
	struct RCparams {
		std::shared_ptr<extras::ArrayBase<double>> RadiusFilter = std::make_shared<extras::Array<double>>(0);
		COM_METHOD COMmethod = MEAN_ABS; //method used for estimating center of mass
		std::shared_ptr<extras::ArrayBase<double>> XYc = std::make_shared<extras::Array<double>>(0); //initial centroid guess
		double DistanceFactor = INFINITY; //radius filter weighting factor
	};
};

namespace extras{namespace ParticleTracking{
	///Radial Center Detection
	///Description:
	/// radialcenter uses Parthasarathy's radial symmetry algorithm to detect origins of asmuthal symmetry in an image
	/// This implementation is capable of using sub-windows to look for multiple particles in the same image
	///
	///Usage:
	/// This function is a template allowing different output types to be specified.
	///
	/// Output argument order is:
	///	X - the x locations of the particle centers
	///	Y - the y locations of the particle centers
	///	varXY - [Nx2] extras::Array of the localization confidence level for X (column 1) and Y (column 2)
	///	RWR/N - the weighted-average distance between the estimated center and all the graident vectors in the sub-image
	template<class OutContainerClass=extras::Array<double>,typename ImageType=double> //OutContainerClass should be class derived from extras::ArrayBase
	std::vector<OutContainerClass> radialcenter(const extras::ArrayBase<ImageType>& I, //input image
												const extras::ArrayBase<double>& WIND, //subwindows to use for finding particles:[x0,y0,w,h]
												const extras::ArrayBase<double>& GP, //gradient noise-sensitivity exponent
												const rcdefs::RCparams& params=rcdefs::RCparams())//parameters
	{
		// Check Input Dimensions and Parameters
		//---------------------------------------
		using namespace std;

		if (!WIND.isempty() && WIND.nCols() != 4) {
			throw(runtime_error("radialcenter: WIND must be empty or Mx4 extras::Array."));
		}
		size_t nPart = max(size_t(1), WIND.nRows());//number of particles to find

		//bool nPart_via_WIND = true; //number of particles determined by windows;
		if(!params.XYc->isempty()){
			if(!WIND.isempty()){
				if(params.XYc->nRows()!=WIND.nRows()){
					throw(std::runtime_error("radialcenter: nRows XYc must match nRows WIND"));
				}
			}
			else{
				//nPart_via_WIND = false;
				nPart = params.XYc->nRows();
			}
		}

		if(params.RadiusFilter->numel()>1){
			if(params.RadiusFilter->numel()!=nPart){
				throw(std::runtime_error("radialcenter: numel RadiusFilter must match number of particles to find"));
			}
		}

		double DF = fabs(params.DistanceFactor); //weighting function distance exponent (if RF==0 w->1/r^DF, else w=1/(1+e^(DF*(r-RF))))



		if (!GP.isempty() && GP.numel() != 1 && GP.numel() != nPart) {
			throw(runtime_error("radialcenter:GP must be empty(default), a scalar value, or the same numel as number of particles."));
		}


		//mexPrintf("about to setup output vars\n");
		//mexEvalString("pause(0.1);");

		//Setup Output variables
		//----------------------------
		vector<OutContainerClass> out;
		out.resize(4);

		auto& x = out[0];
		auto& y = out[1];
		auto& varXY = out[2];
		auto& RWR_N = out[3];


		//mexPrintf("about to resize output\n");
		//mexEvalString("pause(0.1);");

		// Resize output vars
		x.resize(nPart, 1);
		y.resize(nPart, 1);

		varXY.resize(nPart, 2);
		RWR_N.resize(nPart, 1);

		//gradient variables
		extras::Array<double> du;
		extras::Array<double> dv;


		// X and y variables
		// solving x = (X'WX)^-1 X'Wy
		extras::Array<double> sqWX; // sqrt(W)X
		extras::Array<double> sqWy; // sqrt(W)y

		//Extra vars
		extras::Array<double> GradMag;

		// Loop over Particles
		//------------------------------

		size_t Ix1 = 0; //starting x-coord of the window
		size_t Iy1 = 0; //starting y-coord of the window
		size_t Ix2 = I.nCols() - 1;//ending x-coord of the window
		size_t Iy2 = I.nRows() - 1;//ending y-coord of the window
		size_t dNx = Ix2 - Ix1; //width of gradient image
		size_t dNy = Iy2 - Iy1; //height of gradient image

		double gp;
		if (GP.isempty()) {
			gp = DEFAULT_GP;
		}
		else if (GP.numel() == 1) {
			gp = GP[0];
		}

		//mexPrintf("About to enter loop...\n");
		//mexEvalString("pause(0.1);");

		for (size_t n = 0; n < nPart; ++n) {
			//mexPrintf("working on particle %d\n", n);
			//mexEvalString("pause(0.1);");

			double RadFilt = NAN;
			if(params.RadiusFilter->numel()==1){
			RadFilt = fabs((*params.RadiusFilter.get())[0]);
			}else if(params.RadiusFilter->numel()>1){
				RadFilt = fabs((*params.RadiusFilter.get())[n]);
			}

			//Get Sub window range
			bool calc_grad = false;
			size_t newIx1, newIx2,newIy1,newIy2;
			if (!WIND.isempty()) {
				/* Old WIND=[X1,X2,Y1,Y2]
				newIx1 = fmax(0,fmin(I.nCols()-1,floor(WIND(n, 0))));
				newIx2 = fmax(0,fmin(I.nCols()-1,ceil(WIND(n, 1))));
				newIy1 = fmax(0,fmin(I.nRows()-1,floor(WIND(n, 2))));
				newIy2 = fmax(0,fmin(I.nRows()-1,ceil(WIND(n, 3))));
				*/
				/*New WIND=[x0,y0,w,h]*/
				newIx1 = fmax(0,fmin(I.nCols()-1,floor(WIND(n, 0))));
				newIx2 = fmax(0,fmin(I.nCols()-1,ceil(WIND(n, 0)+WIND(n,2)-1)));
				newIy1 = fmax(0,fmin(I.nRows()-1,floor(WIND(n, 1))));
				newIy2 = fmax(0,fmin(I.nRows()-1,ceil(WIND(n, 1)+WIND(n,3)-1)));

				//mexPrintf("Set sub from WIND\n");
				//mexEvalString("pause(0.1)");
			}
			else if(!params.XYc->isempty() && isfinite(RadFilt)&& RadFilt!=0 && DF!=0){ //using region around XY center

				double RadExtents = RadFilt;
				if(isfinite(DF) && DF!=0){ //non-inf Logistic Factor
					double eLRF = exp(DF*RadFilt);
					double eLRFpow= pow(eLRF+1,0.99);
					RadExtents = -( log( 1+eLRF - eLRFpow ) - log(eLRF*eLRFpow) )/DF;
				}
				newIx1 = fmax(0,fmin(I.nCols()-1,floor( (*params.XYc.get())(n,0) - RadExtents )));
				newIx2 = fmax(0,fmin(I.nCols()-1,ceil((*params.XYc.get())(n,0) + RadExtents )));
				newIy1 = fmax(0,fmin(I.nRows()-1,floor((*params.XYc.get())(n,1) - RadExtents )));
				newIy2 = fmax(0,fmin(I.nRows()-1,ceil((*params.XYc.get())(n,1) + RadExtents )));

				//mexPrintf("Set sub from XYc and RadFilt\n");
				//mexEvalString("pause(0.1)");
			}
			else{ //need full frame
				newIx1 = 0; //starting x-coord of the window
				newIy1 = 0; //starting y-coord of the window
				newIx2 = I.nCols() - 1;//ending x-coord of the window
				newIy2 = I.nRows() - 1;//ending y-coord of the window

				//mexPrintf("Set sub to whole image\n");
				//mexEvalString("pause(0.1)");
			}

			if(newIx1!=Ix1 || newIx2!=Ix2 || newIy1!=Iy1 || newIy2!=Iy2){ //check if we need to calc grad again
				Ix1 = newIx1;
				Ix2 = newIx2;
				Iy1 = newIy1;
				Iy2 = newIy2;

				calc_grad = true;
				dNx = Ix2-Ix1;
				dNy = Iy2-Iy1;

				//mexPrintf("Changed wind\n");
				//mexEvalString("pause(0.1)");
			}

			// update the gradient image on first pass, or if we are using sub-windows
			if (n == 0 || calc_grad) {
				//mexPrintf("\t about to calc gradient\n");
				//mexEvalString("pause(0.1);");

				du.resize_nocpy(dNy, dNx);
				dv.resize_nocpy(dNy, dNx);

				//calculate new gradient data
				const ImageType * thisI = &(I(Iy1,Ix1)); //I(y1,x1)
				rcdefs::smoothgrad(thisI, I.nRows(), du.getdata(), dv.getdata(), dNy, dNx);

				sqWX.resize_nocpy(dNx*dNy, 2);
				sqWy.resize_nocpy(dNx*dNy, 1);
			}

			// Determine if we need to calculate COM
			double Xcom,Ycom; //x any y center relative to windows edge
			bool calced_grad_mag = false;
			if(isempty(*params.XYc.get()) ||
			!isfinite((*params.XYc.get())(n,0)) || !isfinite((*params.XYc.get())(n,1)))
			{ //yes, we must calculate COM
				//mexPrintf("Need to calc COM\n");
				//mexEvalString("pause(0.1)");
				switch(params.COMmethod)
				{
					case rcdefs::GRAD_MAG: //COM from magnitude of gradient
					{
						//mexPrintf("about to calc com from grad mag\n");
						//mexEvalString("pause(0.1)");

						GradMag.resize_nocpy(dNy,dNx);
						double grad_acc = 0;
						Xcom = 0;
						Ycom = 0;
						for (size_t yi = 0; yi<dNy; ++yi) {
							double yk = yi + 0.5;
							for (size_t xi = 0; xi<dNx; ++xi) {
								double xk = xi + 0.5;
								GradMag(yi, xi) = sqrt(pow(du(yi, xi), 2) + pow(dv(yi, xi), 2));

								Xcom += xk*GradMag(yi,xi);
								Ycom += yk*GradMag(yi,xi);
								grad_acc += GradMag(yi,xi);
							}
						}
						Xcom /= grad_acc;
						Ycom /= grad_acc;
						calced_grad_mag = true;
					}
					break;
					case rcdefs::MEAN_ABS: //com from absolute of mean-shifted image
					{
						//mexPrintf("about to calc com from mean abs\n");
						//mexEvalString("pause(0.1)");

						//est im mean
						double I_mean = 0;
						double I_acc = 0;
						for (size_t yi = Iy1; yi<=Iy2; ++yi) {
							for (size_t xi = Ix1; xi<=Ix2; ++xi) {
								I_acc+= I(yi,xi);
							}
						}
						I_mean= I_acc/((dNx+1)*(dNy+1));

						Xcom = 0;
						Ycom = 0;
						for (size_t yi = Iy1; yi<=Iy2; ++yi){
							for (size_t xi = Ix1; xi<=Ix2; ++xi) {
								Xcom += (double)xi*fabs( I(yi,xi)-I_mean );
								Ycom += (double)yi*fabs( I(yi,xi)-I_mean );
							}
						}
						Xcom /= I_acc;
						Ycom /= I_acc;
					}
					break;
					case rcdefs::NORMAL: //com from image
					{

						//mexPrintf("about to calc com from normal\n");
						//mexEvalString("pause(0.1)");

						//est im mean
						double I_acc = 0;
						Xcom = 0;
						Ycom = 0;
						for (size_t yi = Iy1; yi<=Iy2; ++yi){
							for (size_t xi = Ix1; xi<=Ix2; ++xi) {
								Xcom += (double)xi*I(yi,xi);
								Ycom += (double)yi*I(yi,xi);
								I_acc+= I(yi,xi);
							}
						}
						Xcom /= I_acc;
						Ycom /= I_acc;
					}
					break;
				}
			}
			else{ //we already have COM
				//mexPrintf("using specified COM\n");
				//mexEvalString("pause(0.1)");

				Xcom = (*params.XYc.get())(n,0)-Ix1;
				Ycom = (*params.XYc.get())(n,1)-Iy1;
				//mexPrintf("\tXc=%f\n",Xcom);
				//mexPrintf("\tYc=%f\n",Ycom);

				if(!isfinite(RadFilt)){
					RadFilt = 0;
					//mexPrintf("Force RadFilt=0\n");
				}
				if(RadFilt==0 && !isfinite(DF)){
					DF = 1;
					//mexPrintf("Force DF=1\n");
				}
			}


			////////////////////////////////
			// Calculate fit
			double sw2 = 0;
			double sw = 0;

			// cov Matrix terms: 1/(AD-B^2)*[D,-B;-B,A]
			double A = 0;
			double B = 0;
			double D = 0;

			double XWy1 = 0;
			double XWy2 = 0;

			if (GP.numel() > 1) {
				gp = GP[n];

				if (!isfinite(gp)){
					gp = DEFAULT_GP;
				}
			}

			if (gp == 0 && !isfinite(RadFilt)) { //no weighting fractor used
				sw2 = dNx*dNy;
				sw = dNx*dNy;

				//mexPrintf("about to calc with no weight factor\n");
				//mexEvalString("pause(0.1)");

				for (size_t yi = 0; yi<dNy; ++yi) {
					double yk = yi + 0.5;
					for (size_t xi = 0; xi<dNx; ++xi) {
						size_t ind = yi + xi*dNy;
						double xk = xi + 0.5;

						double mag = sqrt(pow(du(yi, xi), 2) + pow(dv(yi, xi), 2));
						sqWX(ind, 0) = (du(yi, xi) + dv(yi, xi)) / mag;
						sqWX(ind, 1) = (dv(yi, xi) - du(yi, xi)) / mag;

						sqWy(ind) = xk*sqWX(ind, 0) + yk*sqWX(ind, 1);

						A += sqWX(ind, 0)*sqWX(ind, 0);
						D += sqWX(ind, 1)*sqWX(ind, 1);
						B += sqWX(ind, 0)*sqWX(ind, 1);

						XWy1 += sqWX(ind, 0)*sqWy(ind);
						XWy2 += sqWX(ind, 1)*sqWy(ind);

					}
				}
			}
			else { //use weighting
				//mexPrintf("about to calc with weight factor\n");
				//mexEvalString("pause(0.1)");

				for (size_t yi = 0; yi<dNy; ++yi) {
					double yk = yi + 0.5;
					for (size_t xi = 0; xi<dNx; ++xi) {
						size_t ind = yi + xi*dNy;
						double xk = xi + 0.5;

						//calc mag^2, sqrt(w)/mag, w

						double mag; //magnitude of gradient
						double sqw_mag; //sqrt(w)/mag

						if(calced_grad_mag){
							//mexPrintf("mag from GradMag\n");
							mag = GradMag(yi,xi);
						}
						else{
							//mexPrintf("mag from du dv\n");
							mag = sqrt(pow(du(yi, xi), 2) + pow(dv(yi, xi), 2));
						}

						double w = pow(mag,gp); //weight factor

						// if using radius filter
						if (DF==0){ //no weighting factor
							sqw_mag = pow(mag,gp/2.0-1.0);
						}
						else if(RadFilt==0 && !isfinite(DF)){//no radius and infinite distance factor...should never reach this case
							//w *= 1;
							sqw_mag = pow(mag,gp/2.0-1.0);
						}
						else if(RadFilt==0){//RF=0 && DF>0 use inverse dist
							//mexPrintf("use 1/dist\n");
							double r = sqrt( pow(Xcom-xk,2) + pow(Ycom-yk,2));
							w*= pow(r,-DF);

							if (mag == 0) {
								sqw_mag = 0;
							}
							else {
								sqw_mag = sqrt(w) / mag;
							}

						}
						else if(isfinite(DF)){ //RF>0 DF>0 not inf
							double r = sqrt( pow(Xcom-xk,2) + pow(Ycom-yk,2));
							w*= 1.0/(1.0+exp( DF*(r-RadFilt) ));

							if (mag == 0) {
								sqw_mag = 0;
							}
							else {
								sqw_mag = sqrt(w) / mag;
							}
						}
						else { //RF>0 DF==inf, use a top-hat function as weighting factor
							if(sqrt( pow(Xcom-xk,2) + pow(Ycom-yk,2))>RadFilt){
								w=0;
							}

							if (mag == 0) {
								sqw_mag = 0;
							}
							else {
								sqw_mag = sqrt(w) / mag;
							}
						}

						double w2 = w*w;

						sw2 += w2;
						sw += w;

						sqWX(ind, 0) = (du(yi, xi) + dv(yi, xi))*sqw_mag;
						sqWX(ind, 1) = (dv(yi, xi) - du(yi, xi))*sqw_mag;

						sqWy(ind) = xk*sqWX(ind, 0) + yk*sqWX(ind, 1);

						A += sqWX(ind, 0)*sqWX(ind, 0);
						D += sqWX(ind, 1)*sqWX(ind, 1);
						B += sqWX(ind, 0)*sqWX(ind, 1);

						XWy1 += sqWX(ind, 0)*sqWy(ind);
						XWy2 += sqWX(ind, 1)*sqWy(ind);
					}
				}
			}

			//mexPrintf("solve linear eq\n");
			//mexEvalString("pause(0.1)");

			double det = (A*D - B*B); //calc determinant for inverse

			/*if (det == 0) {
				mexPrintf("Xc: %f Yc%f, det==0\n", Xcom, Ycom);
			}*/

			A /= det;
			B /= det;
			D /= det;
			x[n] = (D*XWy1 - B*XWy2);
			y[n] = (A*XWy2 - B*XWy1);

			/////////////////////////
			// calc variance if needed
			//if (nargout>2) { //calc residual
				double RWR = 0;
				for (int i = 0; i<dNx*dNy; ++i) {
					RWR += pow(sqWy(i) - sqWX(i, 0)*x[n] - sqWX(i, 1)*y[n], 2);
				}

				//calc variance
				double denom = sw*sw / sw2;
				if (denom>2)
					denom -= 2;

				//if (nargout>3) { //calc avg distance from gradient lines
					RWR_N[n] = RWR / (sw - 2 * sw2 / sw);
				//}

				RWR /= denom;

				varXY(n, 0) = D*RWR;
				varXY(n, 1) = A*RWR;

			//}

			x[n] += Ix1;
			y[n] += Iy1;

		}

		// Return output
		return out;
	}
}}
