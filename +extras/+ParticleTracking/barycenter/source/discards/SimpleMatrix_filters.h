#pragma once

#include "SimpleMatrix.h"
#include "SubImage.h"

SimpleMatrix<double> mean3x3(const SubImage& I){
    size_t Ny = I.nRows();
    size_t Nx = I.nCols();

    SimpleMatrix<double> Ot(Ny,Nx);
	// filter along x using 1x3
	for(size_t y=0;y<Ny;++y){
		//handle first column separately
		Ot(y,0) =   (I(y,0) + I(y,1))/2.0;

		//middle no special handling needed
		for(size_t x=1;x<Nx-1;++x){
            Ot(y,x) = (I(y,x-1) + I(y,x) + I(y,x+1))/3.0;
		}

		//handle last column separately
        Ot(y,Nx-1) = (I(y,Nx-2) + I(y,Nx-1))/2.0;
	}

    SimpleMatrix<double> O(Ny,Nx);
	//filter along y using 3x1
	for(size_t x=0;x<Nx;++x){
		//handle first row separately
        O(0,x) =   (Ot(1,x) + I(1,x))/2.0;

		//middle no special handling needed
		for(size_t y=1;y<Ny-1;++y){
            O(y,x) = (Ot(y-1,x) + Ot(y,x) + Ot(y+1,x))/3.0;
		}

		//handle last row separately
        O(Ny-1,x) = (Ot(Ny-2,x) + Ot(Ny-1,x))/2.0;
	}
    return O;
}
