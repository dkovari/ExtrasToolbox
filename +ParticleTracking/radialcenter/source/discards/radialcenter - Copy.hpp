#pragma once

#include <MatrixT.hpp> //MatrixT types

#include <math.h>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>

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
	void mean3x3(const double *I, size_t Ny, size_t Nx, size_t strideI, double* O, size_t strideO) {

		double * Ot = (double*)malloc(Ny*Nx * sizeof(double));
		// filter along x using 1x3
		for (size_t y = 0; y<Ny; ++y) {
			//handle first column separately
			Ot[y] = (I[y] + I[y + strideI * 1]) / 2.0;

			//middle no special handling needed
			for (size_t x = 1; x<Nx - 1; ++x) {
				Ot[y + Ny*(x)] = (I[y + strideI*(x - 1)] + I[y + strideI*(x)] + I[y + strideI*(x + 1)]) / 3.0;
			}

			//handle last column separately
			Ot[y + Ny*(Nx - 1)] = (I[y + strideI*(Nx - 2)] + I[y + strideI*(Nx - 1)]) / 2.0;
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
	void mean3x3(const double *I, size_t Ny, size_t Nx, double* O) {
		mean3x3(I, Ny, Nx, Ny, O, Ny);
	}

	// Calculate 3x3-smoothed image gradient
	// I is colum-major data pointing to I[y1+x1*stride]
	// du and dv should point to pre-allocated arrays of size dNy x dNx
	// corresponding to the window height and width:
	//		dNy = y2-y1; dNx = x2-x1;
	void smoothgrad(const double *I, size_t stride, double *du, double * dv, size_t dNy, size_t dNx) {

		//temp variables
		double *tmp_u, *tmp_v; //finite grad

		tmp_u = (double*)malloc(dNy*dNx * sizeof(double));
		tmp_v = (double*)malloc(dNy*dNx * sizeof(double));

		// calc finite difference
		for (size_t x = 0; x < dNx; ++x) {
			for (size_t y = 0; y < dNy; ++y) {
				tmp_u[y + x*dNy] = I[(y + 1) + (x + 1)*stride] - I[(y)+(x)*stride];
				tmp_v[y + x*dNy] = I[(y + 1) + (x)*stride] - I[(y)+(x + 1)*stride];
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
		MatrixT<double> RadiusFilter = MatrixT<double>(0,0); //radius around the COM (or centroid guess) that should be used
		COM_METHOD COMmethod = MEAN_ABS; //method used for estimating center of mass
		MatrixT<double> XYc = MatrixT<double>(0, 0); //initial centroid guess
		double LogisticRate = NAN; //radius filter weighting factor
	};
};

//Radial Center Detection
//Description:
// radialcenter uses Parthasarathy's radial symmetry algorithm to detect origins of asmuthal symmetry in an image
// This implementation is capable of using sub-windows to look for multiple particles in the same image
//
//Usage:
// This function is a template allowing different output types to be specified.
// The default template type is MatrixT<double>; however any class derived from MatrixT<double> can
// be used as the output type. For example, you can use radialcenter<mexMatrixT<double>> without issue.
//
// You can also specify the number of output arguments, from 1-4.
// Output argument order is:
//	X - the x locations of the particle centers
//	Y - the y locations of the particle centers
//	varXY - [Nx2] array of the localization confidence level for X (column 1) and Y (column 2)
//	RWR/N - the weighted-average distance between the estimated center and all the graident vectors in the sub-image
//
template<class C=MatrixT<double>>
std::vector<C> radialcenter(const MatrixT<double>& I, //input image
											const MatrixT<double>& WIND, //subwindows to use for finding particles: [X1,X2,Y1,Y2]
											const MatrixT<double>& GP, //gradient noise-sensitivity exponent
											const rcdefs::RCparams& params, //optional parameters
											size_t nargout = 4) 
{
	// Check Input Dimensions and Parameters
	//---------------------------------------
	using namespace std;

	nargout = min(nargout, size_t(4));

	bool nPart_via_WIND = true; //number of particles determined by windows;

	if (!WIND.IsEmpty() && WIND.nCols() != 4) {
		throw(runtime_error("radialcenter: WIND must be empty or Mx4 array."));
	}

	if (!params.XYc.IsEmpty()) {
		if (params.XYc.nCols() != 2) {
			throw(runtime_error("radialcenter: params.XYc must have nCols==2 (i.e. X and Y values)"));
		}

		if (!WIND.IsEmpty()) {
			if (params.XYc.nRows() != WIND.nRows()) {
				throw(runtime_error("radialcenter:params.XYc must be empty or have same number of rows as WIND."));
			}
		}
		else {
			nPart_via_WIND = false;
		}
	}

	size_t nPart = max(WIND.nRows(), params.XYc.nRows());

	if (!GP.IsEmpty() && GP.numel() != 1 && GP.numel() != nPart) {
		throw(runtime_error("radialcenter:GP must be empty(default), a scalar value, or the same numel as number of particles."));
	}

	if (params.RadiusFilter.numel()>1) {
		if (nPart != params.RadiusFilter.numel()) {
			throw(runtime_error("radialcenter:params.RadiusFilter must be empty, scalar, or same numel as number of particles (nRows of WIND or XYc)"));
		}
	}

	//Setup Output variables
	//----------------------------
	vector<C> out;
	if (nargout < 1) {
		return out;
	}
	out.resize(4);
	MatrixT<double>& x = out[0];
	MatrixT<double>& y = out[1];
	MatrixT<double>& varXY = out[2];
	MatrixT<double>& RWR_N = out[3];
	

	// Resize output vars
	x.resize(nPart, 1);
	y.resize(nPart, 1);

	if (nargout > 2) {
		varXY.resize(nPart, 2);
	}
	if (nargout > 3) {
		RWR_N.resize(nPart, 1);
	}

	//gradient variables
	MatrixT<double> du;
	MatrixT<double> dv;

	MatrixT<double> Mag; //magnitude of gradient

	// X and y variables
	// solving x = (X'WX)^-1 X'Wy
	MatrixT<double> sqWX; // sqrt(W)X
	MatrixT<double> sqWy; // sqrt(W)y

	// Loop over Particles
	//------------------------------
	
	size_t Ix1 = 0; //starting x-coord of the window
	size_t Iy1 = 0; //starting y-coord of the window
	size_t Ix2 = I.nCols() - 1;//ending x-coord of the window
	size_t Iy2 = I.nRows() - 1;//ending y-coord of the window
	size_t dNx = Ix2 - Ix1; //width of gradient image
	size_t dNy = Iy2 - Iy1; //height of gradient image

	double gp;
	if (GP.IsEmpty()) {
		gp = DEFAULT_GP;
	}
	else if (GP.numel() == 1) {
		gp = GP[0];
	}

	for (size_t n = 0; n < nPart; ++n) {
		if (!WIND.IsEmpty()) {
			Ix1 = fmax(0,fmin(I.nCols()-1,floor(WIND(n, 0))));
			Ix2 = fmax(0,fmin(I.nCols()-1,ceil(WIND(n, 1))));
			Iy1 = fmax(0,fmin(I.nRows()-1,floor(WIND(n, 2))));
			Iy2 = fmax(0,fmin(I.nRows()-1,ceil(WIND(n, 3))));

			dNx = WIND(n, 1) - WIND(n, 0);
			dNy = WIND(n, 3) - WIND(n, 2);
		}

		// update the gradient image on first pass, or if we are using sub-windows
		if (n == 0 || !WIND.IsEmpty()) { 
			du.resize_nocpy(dNy, dNx);
			dv.resize_nocpy(dNy, dNx);

			//calculate new gradient data
			const double * thisI = &(I(Iy1,Ix1)); //I(y1,x1)
			rcdefs::smoothgrad(thisI, I.nRows(), du.getdata(), dv.getdata(), dNy, dNx);

			sqWX.resize_nocpy(dNx*dNy, 2);
			sqWy.resize_nocpy(dNx*dNy, 1);
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
		}

		//determine if center of mass needs to be estimated
		double thisRF;
		if (params.RadiusFilter.IsEmpty()) {
			thisRF = NAN;
		}
		else if(numel(params.RadiusFilter) == 1){
			thisRF = params.RadiusFilter[0];
		}
		else {
			thisRF = params.RadiusFilter[n];
		}

		if (thisRF == 0)
		{
			thisRF = -1;
		}

		bool calcCOM = (params.XYc.IsEmpty() || !isfinite(params.XYc(n,0)) || !isfinite(params.XYc(n,1)))&& isfinite(thisRF);
		
		//calc radial filter center points
		double rfx = 0;
		double rfy = 0;
		if (calcCOM) {
			switch (params.COMmethod) {
			case rcdefs::MEAN_ABS:
				{
					double mI = 0; //mean of I
					double sI = 0; //sum of I
					for (size_t x = Ix1; x <= Ix2; ++x) {
						for (size_t y = Iy1; y <= Iy2; ++y) {
							mI += I(y,x);
						}
					}
					mI /= (dNx + 1)*(dNy + 1);

					for (size_t x = Ix1; x <=Ix2; ++x) {
						for (size_t y =Iy1; y <=Iy2; ++y) {
							double absIm = fabs(I(y,x) - mI);
							rfx += double(x + 1)*absIm;
							rfy += double(y + 1)*absIm;
							sI += absIm;
						}
					}
					rfx /= sI;
					rfy /= sI;
				}
				break;
			case rcdefs::NORMAL:
				{
				double sI = 0; //sum of I
				for (size_t x =Ix1; x <=Ix2; ++x) {
					for (size_t y = Iy1; y <= Iy2; ++y) {
						rfx += double(x + 1)*I(y, x);
						rfy += double(y + 1)*I(y, x);
						sI += I(y, x);
					}
				}
				rfx /= sI;
				rfy /= sI;
				}
				break;
			case rcdefs::GRAD_MAG:
				if(n==0||WIND.numel()>1){
				double sG = 0; //sum of |grad(i)|
				Mag.resize_nocpy(dNy, dNx);
				for (int xi = 0; xi<dNx; ++xi) {
					double xk = xi + 1.5;
					for (int yi = 0; yi<dNy; ++yi) {
						double yk = yi + 1.5;

						Mag(yi, xi) = sqrt(pow(du(yi, xi), 2) + pow(dv(yi, xi), 2));
						rfx += xk*Mag(yi, xi);
						rfy += yk*Mag(yi, xi);
						sG += Mag(yi, xi);
					}
				}
				rfx /= sG;
				rfy /= sG;

				rfx += Ix1; //shift back to window range
				rfy += Iy1;
				}
				break;
			}

		}
		else if(!params.XYc.IsEmpty()){
			rfx = params.XYc(n, 0);
			rfy = params.XYc(n, 1);
		}
		rfx -= Ix1;
		rfy -= Iy1;

		if (gp == 0 && !isfinite(thisRF)) { //no weighting fractor used
			sw2 = dNx*dNy;
			sw = dNx*dNy;

			for (size_t yi = 0; yi<dNy; ++yi) {
				double yk = yi + 1.5;
				for (size_t xi = 0; xi<dNx; ++xi) {
					size_t ind = yi + xi*dNy;
					double xk = xi + 1.5;

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
			for (size_t yi = 0; yi<dNy; ++yi) {
				double yk = yi + 1.5;
				for (size_t xi = 0; xi<dNx; ++xi) {

					size_t ind = yi + xi*dNy;
					double xk = xi + 1.5;

					//calc mag^2
					double mag;
					if (calcCOM && params.COMmethod == rcdefs::GRAD_MAG) { //already calculated the gradient mag
						mag = Mag(yi, xi);
					}
					else {
						mag = sqrt(pow(du(yi, xi), 2) + pow(dv(yi, xi), 2));
					}

					double w = pow(mag, gp);

					if (isfinite(thisRF)) { //use a radius filter
						double r = sqrt(pow(rfx - xk, 2) + pow(rfy - yk, 2));

						if (thisRF > 0) { //use radius threshold
							if (isfinite(params.LogisticRate)) { //use logistic filter
								w *= double(1) / (1 + exp(params.LogisticRate*(r - thisRF)));
							}
							else if (r > thisRF) {
								w = 0;
							}
						}
						else{ //use distance weighting
							w *= double(1) / pow(r, -thisRF);
						}
					}


					double w2 = w*w;
					double sqw_mag = pow(mag, gp / 2 - 1);

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

		double det = (A*D - B*B); //calc determinant for inverse
		A /= det;
		B /= det;
		D /= det;
		x[n] = (D*XWy1 - B*XWy2);
		y[n] = (A*XWy2 - B*XWy1);

		/////////////////////////
		// calc variance if needed
		if (nargout>2) {
			//calc residual
			double RWR = 0;
			for (int i = 0; i<dNx*dNy; ++i) {
				RWR += pow(sqWy(i) - sqWX(i, 0)*x[n] - sqWX(i, 1)*y[n], 2);
			}

			//calc variance
			double denom = sw*sw / sw2;
			if (denom>2)
				denom -= 2;

			if (nargout>3) {
				RWR_N[n] = RWR / (sw - 2 * sw2 / sw);
			}

			RWR /= denom;

			varXY(n, 0) = D*RWR;
			varXY(n, 1) = A*RWR;

		}

		x[n] += Ix1;
		y[n] += Iy1;

	}

	// Return output
	out.resize(nargout);
	return out;
}


std::vector<MatrixT<double>> radialcenter(const MatrixT<double>& I, const MatrixT<double>& WIND, const MatrixT<double> GP, MatrixT<double> RFsize, rcdefs::COM_METHOD COMmethod, size_t nargout = 4) {
	rcdefs::RCparams params;
	params.RadiusFilter = std::move(RFsize);
	params.COMmethod = COMmethod;

	return radialcenter<MatrixT<double>>(I, WIND, GP, params, nargout);
}

std::vector<MatrixT<double>> radialcenter(const MatrixT<double>& I, const MatrixT<double>& WIND, const MatrixT<double>& GP, MatrixT<double> RFsize, size_t nargout = 4) {
	return radialcenter(I, WIND, GP,std::move(RFsize),rcdefs::MEAN_ABS, nargout);
}



