#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>

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
template<typename T>
void mean3x3(const T *I, size_t Ny, size_t Nx, size_t strideI, double* O, size_t strideO){

    double * Ot = (double*)malloc(Ny*Nx * sizeof(double));
	// filter along x using 1x3
	for(size_t y=0;y<Ny;++y){
		//handle first column separately
		Ot[y] = (I[y]+I[y+strideI*1])/2.0;

		//middle no special handling needed
		for(size_t x=1;x<Nx-1;++x){
			Ot[y+Ny*(x)] = (I[y+strideI*(x-1)] + I[y+strideI*(x)] +I[y+strideI*(x+1)])/3.0;
		}

		//handle last column separately
		Ot[y+Ny*(Nx-1)] = (I[y+strideI*(Nx-2)] + I[y+strideI*(Nx-1)] )/2.0;
	}

	//filter along y using 3x1
	for(size_t x=0;x<Nx;++x){
		//handle first row separately
		O[strideO*x] = (Ot[Ny*x] + Ot[1+Ny*x])/2.0;

		//middle no special handling needed
		for(size_t y=1;y<Ny-1;++y){
			O[y+strideO*(x)] = (Ot[y-1+Ny*(x)] + Ot[y+Ny*(x)] +Ot[y+1+Ny*(x)])/3.0;
		}

		//handle last row separately
		O[Ny-1+strideO*x] = (Ot[Ny-2+Ny*x] + Ot[Ny-1+Ny*x])/2.0;
	}

    free(Ot);

}

// Alias to mean3x3(...)
// calls mean3x3 w/ strideI=stride)=Ny
void mean3x3(const double *I, size_t Ny, size_t Nx, double* O){
	mean3x3(I,Ny,Nx,Ny,O,Ny);
}
