#pragma once

/*
Version of radial center which does not use extras::ArrayBase class
*/

#include <math.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <memory>

#include <extras/assert.hpp>

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

    /*template<typename M>
    const M& getElement(const M* data, size_t stride, size_t r, size_t c){
        return data[r+c*stride];
    }
    template<typename M>
    M& getElement(M* data, size_t stride, size_t r, size_t c){
        return data[r+c*stride];
    }*/
}

namespace extras{ namespace ParticleTracking{

    struct RadialcenterParameters{
        double default_RadiusCutoff = INFINITY;
        double default_CutoffFactor = INFINITY;
        double default_DistanceExponent = 1;
        double default_GradientExponent = 5;

        double* WIND = nullptr;
        size_t nWIND = 0;
        double* XYc = nullptr;
        size_t nXYc = 0;
        double* RadiusCutoff = &default_RadiusCutoff;
        size_t nRadiusCutoff = 1;
        double* CutoffFactor = &default_CutoffFactor;
        size_t nCutoffFactor = 1;
        rcdefs::COM_METHOD COMmethod = rcdefs::GRAD_MAG;
        double* DistanceExponent = &default_DistanceExponent;
        size_t nDistanceExponent = 1;
        double* GradientExponent = &default_GradientExponent;
        size_t nGradientExponent = 1;

        RadialcenterParameters() = default;
        RadialcenterParameters(const RadialcenterParameters&) = default;
        RadialcenterParameters(RadialcenterParameters&&) = default;
        RadialcenterParameters& operator=(const RadialcenterParameters&) = default;
        RadialcenterParameters& operator=(RadialcenterParameters&&) = default;

    };

    template <typename M>
    void radialcenter(double* x, double* y, double* varXY, double* RWR_N, //pointers to 4 arrays of appropriate size
        const M* img, size_t nRows, size_t nCols, //image and size (assume column-major indexing)
        const RadialcenterParameters& params = RadialcenterParameters() //parameters
        )
    {
        // Check Input Dimensions and Parameters
		//---------------------------------------
		using namespace std;
        using namespace rcdefs;

        size_t nPart = max(size_t(1), params.nWIND); //number of windows

        // check for XYc
		if(params.nXYc != 0){
			if(params.nWIND!=0){
				if(params.nXYc!=params.nWIND){
					throw(std::runtime_error("radialcenter: nRows XYc must match nRows WIND"));
				}
			}
			else{
				nPart = params.nXYc;
			}
		}

        // validate RadiusCutoff
        if (params.nRadiusCutoff ==0) { //empty RadiusCutoff
            throw("Empty RadiusCutoff not supported");
		}
		else if (params.nRadiusCutoff ==1) { //
		}
		else { //make sure same size as nPart
			extras::assert_condition(params.nRadiusCutoff == nPart,"RadiusCutoff has wrong number of elements");
		}

        if (params.nCutoffFactor ==0) { //empty RadiusCutoff
            throw("Empty CutoffFactor not supported");
		}
		else if (params.nCutoffFactor ==1) { //
		}
		else { //make sure same size as nPart
			extras::assert_condition(params.nCutoffFactor == nPart,"CutoffFactor has wrong number of elements");
		}

        if (params.nDistanceExponent ==0) { //empty RadiusCutoff
            throw("Empty DistanceExponent not supported");
		}
		else if (params.nDistanceExponent ==1) { //
		}
		else { //make sure same size as nPart
			extras::assert_condition(params.nDistanceExponent == nPart,"DistanceExponent has wrong number of elements");
		}

        if (params.nGradientExponent ==0) { //empty RadiusCutoff
            throw("Empty GradientExponent not supported");
		}
		else if (params.nGradientExponent ==1) { //
		}
		else { //make sure same size as nPart
			extras::assert_condition(params.nGradientExponent == nPart,"GradientExponent has wrong number of elements");
		}

        // Gradient variables
        double * du = nullptr;
        double * dv = nullptr;

        double *sqWX = nullptr;
        double *sqWy = nullptr;

        double* GradMag = nullptr;
        bool calced_grad_mag = false;

        size_t Ix1 = 0; //starting x-coord of the window
		size_t Iy1 = 0; //starting y-coord of the window
		size_t Ix2 = nCols - 1;//ending x-coord of the window
		size_t Iy2 = nRows - 1;//ending y-coord of the window
		size_t dNx = Ix2 - Ix1; //width of gradient image
		size_t dNy = Iy2 - Iy1; //height of gradient image

        // loop over particles and compute
        for (size_t n = 0; n < nPart; ++n) {
            ///////////////////////////
			// Parameters

			double this_RadiusCutoff;
			if (params.nRadiusCutoff==0) {
				this_RadiusCutoff = params.default_RadiusCutoff;
			}
            else if(params.nRadiusCutoff==1){
                this_RadiusCutoff = params.RadiusCutoff[0];
            }
			else {
				this_RadiusCutoff = params.RadiusCutoff[n];
			}
			assert_condition(this_RadiusCutoff >= 0, "RadiusCutoff must be >=0");

			double this_CutoffFactor;
            if (params.nCutoffFactor==0) {
				this_CutoffFactor = params.default_CutoffFactor;
			}
            else if(params.nCutoffFactor==1){
                this_CutoffFactor = params.CutoffFactor[0];
            }
			else {
				this_CutoffFactor = params.CutoffFactor[n];
			}
			assert_condition(this_CutoffFactor >= 0, "CutoffFactor must be >=0");
			if (this_CutoffFactor == 0) {
				this_RadiusCutoff = INFINITY;
			}

			double this_DistanceExponent;
            if (params.nDistanceExponent==0) {
				this_DistanceExponent = params.default_DistanceExponent;
			}
            else if(params.nDistanceExponent==1){
                this_DistanceExponent = params.DistanceExponent[0];
            }
			else {
				this_DistanceExponent = params.DistanceExponent[n];
			}

			double this_GradientExponent;
            if (params.nGradientExponent==0) {
				this_GradientExponent = params.default_GradientExponent;
			}
            else if(params.nGradientExponent==1){
                this_GradientExponent = params.GradientExponent[0];
            }
			else {
				this_GradientExponent = params.GradientExponent[n];
			}

            //////////////////////////////////
			//Get Sub window range
			bool calc_grad = false; //flag saying gradient needs to be calculated
			size_t newIx1, newIx2,newIy1,newIy2;
			if (params.nWIND!=0) { //we have window
				/* Old WIND=[X1,X2,Y1,Y2]
				newIx1 = fmax(0,fmin(I.nCols()-1,floor(WIND(n, 0))));
				newIx2 = fmax(0,fmin(I.nCols()-1,ceil(WIND(n, 1))));
				newIy1 = fmax(0,fmin(I.nRows()-1,floor(WIND(n, 2))));
				newIy2 = fmax(0,fmin(I.nRows()-1,ceil(WIND(n, 3))));
				*/
				/*New WIND=[x0,y0,w,h]*/
				newIx1 = fmax(0,fmin(nCols-1,floor( params.WIND[n+0*params.nWIND])));//WIND(n, 0))));
				newIx2 = fmax(0,fmin(nCols-1,ceil(params.WIND[n+0*params.nWIND] + params.WIND[n+2*params.nWIND] -1)));///WIND(n, 0)+WIND(n,2)-1)));
				newIy1 = fmax(0,fmin(nRows-1,floor(params.WIND[n+1*params.nWIND])));///WIND(n, 1))));
				newIy2 = fmax(0,fmin(nRows-1,ceil(params.WIND[n+1*params.nWIND]+params.WIND[n+3*params.nWIND]-1)));///WIND(n, 1)+WIND(n,3)-1)));
			}
			else if(params.nXYc!=0 && isfinite(this_RadiusCutoff)&& this_RadiusCutoff !=0){ //using region around XY center

				double RadExtents = this_RadiusCutoff;
				if(isfinite(this_CutoffFactor)){ //non-inf Logistic Factor
					double eLRF = exp(this_CutoffFactor*this_RadiusCutoff);
					double eLRFpow= pow(eLRF+1,0.99);
					RadExtents = -( log( 1+eLRF - eLRFpow ) - log(eLRF*eLRFpow) )/ this_CutoffFactor;
				}
				newIx1 = fmax(0,fmin(nCols-1,floor( params.XYc[n+0*params.nXYc] - RadExtents)));//(*params.XYc.get())(n,0) - RadExtents )));
				newIx2 = fmax(0,fmin(nCols-1,ceil( params.XYc[n+0*params.nXYc] + RadExtents)));///(*params.XYc.get())(n,0) + RadExtents )));
				newIy1 = fmax(0,fmin(nRows-1,floor(params.XYc[n+1*params.nXYc] - RadExtents)));///(*params.XYc.get())(n,1) - RadExtents )));
				newIy2 = fmax(0,fmin(nRows-1,ceil(params.XYc[n+1*params.nXYc] + RadExtents)));///*params.XYc.get())(n,1) + RadExtents )));
			}
			else{ //need full frame
				newIx1 = 0; //starting x-coord of the window
				newIy1 = 0; //starting y-coord of the window
				newIx2 = nCols - 1;//ending x-coord of the window
				newIy2 = nRows - 1;//ending y-coord of the window

			}

			if(newIx1!=Ix1 || newIx2!=Ix2 || newIy1!=Iy1 || newIy2!=Iy2){ //check if we need to calc grad again
				Ix1 = newIx1;
				Ix2 = newIx2;
				Iy1 = newIy1;
				Iy2 = newIy2;

				calc_grad = true;
				dNx = Ix2-Ix1;
				dNy = Iy2-Iy1;
			}

			// update the gradient image on first pass, or if we are using sub-windows
			if (n == 0 || calc_grad) {

                //resize du
				du = (double*)std::realloc(du, dNy*dNx * sizeof(double));

                //resize dv
				dv = (double*)std::realloc(dv, dNy*dNx * sizeof(double));

				//calculate new gradient data
				const M * thisI = &( img[Iy1+Ix1*nRows]); //I(Iy1,Ix1)); //I(y1,x1)
				rcdefs::smoothgrad(thisI, nRows, du, dv, dNy, dNx);
                calc_grad = false;

				std::free(GradMag);
				GradMag = nullptr;
                calced_grad_mag = false;


                //resize sqWX
				sqWX = (double*)std::realloc(sqWX, 2*dNy*dNx * sizeof(double));

                //resize sqWy
				sqWy = (double*)std::realloc(sqWy, dNy*dNx * sizeof(double));
			}

			// Determine if we need to calculate COM
			double Xcom, Ycom;//x any y center relative to windows edge
			bool need_COM = true;


			if (this_DistanceExponent == 0 && ((this_RadiusCutoff == 0 || !isfinite(this_RadiusCutoff)) || this_CutoffFactor == 0)) { //dont need COM because we aren't using distance dependence
				//nothing to do
				need_COM = false;
			}
			else if( params.nXYc != 0 && isfinite(params.XYc[n+0*params.nXYc]) && isfinite(params.XYc[n+1*params.nXYc])){ //dont need because we were valid told XYc
				Xcom = params.XYc[n+0*params.nXYc]-Ix1;//params.XYc->getElement(n, 0) - Ix1;
				Ycom = params.XYc[n+1*params.nXYc]-Iy1;//params.XYc->getElement(n, 1) - Iy1;
				need_COM = true;
			}
			else { //need to calculate COM
				need_COM = true;
				switch (params.COMmethod)
				{
				case rcdefs::GRAD_MAG: //COM from magnitude of gradient
				{
					//GradMag.resize_nocpy(dNy, dNx);
                    if(!calced_grad_mag){
						GradMag = (double*)std::realloc(GradMag, dNy*dNx * sizeof(double));
                    }

					double grad_acc = 0;
					Xcom = 0;
					Ycom = 0;
					for (size_t yi = 0; yi<dNy; ++yi) {
						double yk = yi + 0.5;
						for (size_t xi = 0; xi<dNx; ++xi) {
							double xk = xi + 0.5;
                            size_t ind = yi + xi*dNy;
                            if(!calced_grad_mag){
                                GradMag[ind]= sqrt( pow(du[ind],2) + pow(dv[ind],2) ); //GradMag(yi, xi) = sqrt(pow(du(yi, xi), 2) + pow(dv(yi, xi), 2));
                            }

							Xcom += xk * GradMag[ind];//GradMag(yi, xi);
							Ycom += yk * GradMag[ind];//GradMag(yi, xi);
							grad_acc += GradMag[ind];//GradMag(yi, xi);
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
					for (size_t yi = Iy1; yi <= Iy2; ++yi) {
						for (size_t xi = Ix1; xi <= Ix2; ++xi) {

							I_acc += img[yi*nRows*xi];//I(yi, xi);
						}
					}
					I_mean = I_acc / ((dNx + 1)*(dNy + 1));

					Xcom = 0;
					Ycom = 0;
					for (size_t yi = Iy1; yi <= Iy2; ++yi) {
						for (size_t xi = Ix1; xi <= Ix2; ++xi) {
                            double i_m = img[yi*nRows*xi] - I_mean;
							Xcom += (double)xi*i_m;//I(yi, xi) - I_mean);
							Ycom += (double)yi*i_m;//I(yi, xi) - I_mean);
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
					for (size_t yi = Iy1; yi <= Iy2; ++yi) {
						for (size_t xi = Ix1; xi <= Ix2; ++xi) {
							Xcom += (double)xi*img[yi*nRows*xi];//I(yi, xi);
							Ycom += (double)yi*img[yi*nRows*xi];//I(yi, xi);
							I_acc += img[yi*nRows*xi];//I(yi, xi);
						}
					}
					Xcom /= I_acc;
					Ycom /= I_acc;
				}
				break;
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

			//setup weight factor
			if (this_GradientExponent == 0 && !isfinite(this_RadiusCutoff) && this_DistanceExponent==0) { //no weighting fractor used
				sw2 = dNx*dNy;
				sw = dNx*dNy;

				for (size_t yi = 0; yi<dNy; ++yi) {
					double yk = yi + 0.5;
					for (size_t xi = 0; xi<dNx; ++xi) {
						size_t ind = yi + xi*dNy;
						double xk = xi + 0.5;

                        double mag;
                        if(!calced_grad_mag){
                            mag= sqrt( pow(du[ind],2) + pow(dv[ind],2) );

                        }else{
                            mag = GradMag[ind];
                        }

						sqWX[ind+dNy*dNx*0] = (du[ind] + dv[ind])/mag;// //sqWX(ind, 0) = (du(yi, xi) + dv(yi, xi)) / mag;
						sqWX[ind+dNy*dNx*1] = (dv[ind] - du[ind])/mag;//sqWX(ind, 0) = (dv(yi, xi) - du(yi, xi)) / mag;

						sqWy[ind] = xk*sqWX[ind+dNy*dNx*0] + yk*sqWX[ind+dNy*dNx*1];//xk*sqWX(ind, 0) + yk*sqWX(ind, 1);

						A += pow(sqWX[ind+dNy*dNx*0],2);//sqWX(ind, 0)*sqWX(ind, 0);
						D += pow(sqWX[ind+dNy*dNx*1],2);//sqWX(ind, 1)*sqWX(ind, 1);
						B += sqWX[ind+dNy*dNx*0]*sqWX[ind+dNy*dNx*1];//sqWX(ind, 0)*sqWX(ind, 1);

						XWy1 += sqWX[ind+dNy*dNx*0]*sqWy[ind];//sqWX(ind, 0)*sqWy(ind);
						XWy2 += sqWX[ind+dNy*dNx*1]*sqWy[ind];//qWX(ind, 1)*sqWy(ind);
					}
				}
			}
			else { //use weighting
				for (size_t yi = 0; yi<dNy; ++yi) {
					double yk = yi + 0.5;
					for (size_t xi = 0; xi<dNx; ++xi) {
						size_t ind = yi + xi*dNy;
						double xk = xi + 0.5;

						//calc mag^2, sqrt(w)/mag, w
						double sqw_mag; //sqrt(w)/mag
                        double mag;
                        if(!calced_grad_mag){
                            mag= sqrt( pow(du[ind],2) + pow(dv[ind],2) );

                        }else{
                            mag = GradMag[ind];
                        }

						double w = pow(mag, this_GradientExponent); //weight factor

						// Apply distance weight
						if (!isfinite(this_RadiusCutoff) && this_DistanceExponent==0) { //no distance dependence
							//do nothing
						}
						else { //has distance dependence
							double this_r = sqrt(pow(Xcom - xk, 2) + pow(Ycom - yk, 2));
							if (!isfinite(this_CutoffFactor) && this_r > this_RadiusCutoff) { //use top-hat filter and outside cutoff
								w = 0;
							}
							else {
								if (this_DistanceExponent != 0) {
									w /= pow(this_r, this_DistanceExponent);
								}
								if (isfinite(this_CutoffFactor)) { //using logistic filter
									w *= 1.0 / (1.0 + exp(this_CutoffFactor*(this_r - this_RadiusCutoff)));
								}
							}
						}
						if (mag == 0) { //special case when gradient was exactly zero (probably rare)
							sqw_mag = 0;
						}
						else {
							sqw_mag = sqrt(w) / mag;
						}

						double w2 = w*w;

						sw2 += w2;
						sw += w;

						sqWX[ind+dNy*dNx*0] = (du[ind] + dv[ind])*sqw_mag;//sqWX(ind, 0) = (du(yi, xi) + dv(yi, xi))*sqw_mag;
						sqWX[ind+dNy*dNx*1] = (dv[ind] - du[ind])*sqw_mag;//sqWX(ind, 1) = (dv(yi, xi) - du(yi, xi))*sqw_mag;

						/*sqWy(ind) = xk*sqWX(ind, 0) + yk*sqWX(ind, 1);

						A += sqWX(ind, 0)*sqWX(ind, 0);
						D += sqWX(ind, 1)*sqWX(ind, 1);
						B += sqWX(ind, 0)*sqWX(ind, 1);

						XWy1 += sqWX(ind, 0)*sqWy(ind);
						XWy2 += sqWX(ind, 1)*sqWy(ind);*/

						sqWy[ind] = xk*sqWX[ind+dNy*dNx*0] + yk*sqWX[ind+dNy*dNx*1];//xk*sqWX(ind, 0) + yk*sqWX(ind, 1);

						A += pow(sqWX[ind+dNy*dNx*0],2);//sqWX(ind, 0)*sqWX(ind, 0);
						D += pow(sqWX[ind+dNy*dNx*1],2);//sqWX(ind, 1)*sqWX(ind, 1);
						B += sqWX[ind+dNy*dNx*0]*sqWX[ind+dNy*dNx*1];//sqWX(ind, 0)*sqWX(ind, 1);

						XWy1 += sqWX[ind+dNy*dNx*0]*sqWy[ind];//sqWX(ind, 0)*sqWy(ind);
						XWy2 += sqWX[ind+dNy*dNx*1]*sqWy[ind];//qWX(ind, 1)*sqWy(ind);
					}
				}
			}


			double det = (A*D - B*B); //calc determinant for inverse

			A /= det;
			B /= det;
			D /= det;
			x[n] = (D*XWy1 - B*XWy2);
			y[n] = (A*XWy2 - B*XWy1);

			/////////////////////////
			// calc variance

			double RWR = 0;
			for (int i = 0; i<dNx*dNy; ++i) {
				RWR += pow(sqWy[i] - sqWX[i+dNy*dNx*0]*x[n] - sqWX[i+dNy*dNx*1]*y[n], 2);
			}

			//calc variance
			double denom = sw*sw / sw2;
			if (denom>2)
				denom -= 2;

			RWR_N[n] = RWR / (sw - 2 * sw2 / sw);

			RWR /= denom;

			varXY[n+nPart*0] = D*RWR;
			varXY[n+nPart*1] = A*RWR;

			x[n] += Ix1;
			y[n] += Iy1;

		}


		///////////
		// Cleanup memory
		std::free(du);
		std::free(dv);
		std::free(sqWX);
		std::free(sqWy);
		std::free(GradMag);

    }
}}
