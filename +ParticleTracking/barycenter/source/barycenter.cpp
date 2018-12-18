/*
[X,Y] = BaryCenter(Image, WIND, Sz, LimFrac)
Input:
    Image: 2D matrix with image data
    WIND: [n x 4] array specifying subwindows to process
        if not specified, defaults to entire image
    Sz: Narrow boundary around max/min of each sub-image to process
        if not specified, defualts to entire sub-image
    LimFrac: Threshold factor for selecting brightest and darkest regions
        default=0.2
        Threshold is calculated as:
            Low = Range*LimFrac + min
            UP = Range*(1-LimFrac) + min
Outputs:
    [X,Y] coordinates for each window
        if something went wrong returns NaN
*/

//#define DEBUG_PLOT

#define DEFAULT_LIM_FRAC 0.2

#include <mex.h>
#include "SimpleMatrix.h"
#include <cmath>
#include <list>
#include "MatrixT.hpp"
#include <string>
#include "SubImage.h"
#include <vector>

#ifdef GI_P5X5
#define USE_GI
#elif GI_M5X5
#define USE_GI
#elif GI_M3X3
#define USE_GI
#endif



template<typename T>
inline T vmax(T a, T b){
	return (a >= b) ? a : b;
}

template<typename T>
inline T vmin(T a, T b){
	return (a <= b) ? a : b;
	//return a; //this is a mistake
}

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
        O(0,x) =   (Ot(0,x) + Ot(1,x))/2.0;

		//middle no special handling needed
		for(size_t y=1;y<Ny-1;++y){
            O(y,x) = (Ot(y-1,x) + Ot(y,x) + Ot(y+1,x))/3.0;
		}

		//handle last row separately
        O(Ny-1,x) = (Ot(Ny-2,x) + Ot(Ny-1,x))/2.0;
	}
    return O;
}

SimpleMatrix<double> mean5x5(const SubImage& I){
    size_t Ny = I.nRows();
    size_t Nx = I.nCols();

    SimpleMatrix<double> Ot(Ny,Nx);
	// filter along x using 1x5
	for(size_t y=0;y<Ny;++y){
		//handle first column separately
		Ot(y,0) =   (I(y,0) + I(y,1) + I(y,2))/3.0;

		//handle second column separately
		Ot(y,1) =   (I(y,0) + I(y,1) + I(y,2) + I(y,3))/4.0;

		//middle no special handling needed
		for(size_t x=2;x<Nx-2;++x){
            Ot(y,x) = ( I(y,x-2) + I(y,x-1) + I(y,x) + I(y,x+1) + I(y,x+2) )/5.0;
		}

		//handle second to last column separately
		Ot(y,Nx-2) =   (I(y,Nx-4) + I(y,Nx-3) + I(y,Nx-2) + I(y,Nx-1))/4.0;
		//handle last column separately
        Ot(y,Nx-1) = (I(y,Nx-3) + I(y,Nx-2) + I(y,Nx-1))/3.0;
	}

    SimpleMatrix<double> O(Ny,Nx);
	//filter along y using 5x1
	for(size_t x=0;x<Nx;++x){
		//handle first row separately
        O(0,x) =   (Ot(0,x) + Ot(1,x) + Ot(2,x))/3.0;

		//handle second row separately
        O(1,x) =   (Ot(0,x) + Ot(1,x) + Ot(2,x)+ Ot(3,x))/4.0;

		//middle no special handling needed
		for(size_t y=2;y<Ny-2;++y){
            O(y,x) = (Ot(y-2,x) + Ot(y-1,x) + Ot(y,x) + Ot(y+1,x) + Ot(y+2,x))/5.0;
		}
		//handle second to last row separately
        O(Ny-2,x) = (Ot(Ny-4,x) +Ot(Ny-3,x) +Ot(Ny-2,x) + Ot(Ny-1,x))/4.0;

		//handle last row separately
        O(Ny-1,x) = (Ot(Ny-3,x) + Ot(Ny-2,x) + Ot(Ny-1,x))/3.0;
	}
    return O;
}

SimpleMatrix<double> parab5x5(const SubImage& I){
    size_t Ny = I.nRows();
    size_t Nx = I.nCols();

    SimpleMatrix<double> Ot(Ny,Nx);
	// filter along x using 1x5
	for(size_t y=0;y<Ny;++y){
		//handle first column separately
		Ot(y,0) =   (9*I(y,0) + 8*I(y,1) + 5*I(y,2))/(9+8+5);

		//handle second column separately
		Ot(y,1) =   (8*I(y,0) + 9*I(y,1) + 8*I(y,2) + 5*I(y,3))/(8+9+8+5);

		//middle no special handling needed
		for(size_t x=2;x<Nx-2;++x){
            Ot(y,x) = ( 5*I(y,x-2) + 8*I(y,x-1) + 9*I(y,x) + 8*I(y,x+1) + 5*I(y,x+2) )/(5+8+9+8+5);
		}

		//handle second to last column separately
		Ot(y,Nx-2) =   (5*I(y,Nx-4) + 8*I(y,Nx-3) + 9*I(y,Nx-2) + 8*I(y,Nx-1))/(5+8+9+8);
		//handle last column separately
        Ot(y,Nx-1) = (5*I(y,Nx-3) + 8*I(y,Nx-2) + 9*I(y,Nx-1))/(5+8+9);
	}

    SimpleMatrix<double> O(Ny,Nx);
	//filter along y using 5x1
	for(size_t x=0;x<Nx;++x){
		//handle first row separately
        O(0,x) =   (9*Ot(0,x) + 8*Ot(1,x) + 5*Ot(2,x))/(9+8+5);

		//handle second row separately
        O(1,x) =   (8*Ot(0,x) + 9*Ot(1,x) + 8*Ot(2,x)+ 5*Ot(3,x))/(8+9+8+5);

		//middle no special handling needed
		for(size_t y=2;y<Ny-2;++y){
            O(y,x) = (5*Ot(y-2,x) + 8*Ot(y-1,x) + 9*Ot(y,x) + 8*Ot(y+1,x) + 5*Ot(y+2,x))/(5+8+9+8+5);
		}
		//handle second to last row separately
        O(Ny-2,x) = (5*Ot(Ny-4,x) +8*Ot(Ny-3,x) +9*Ot(Ny-2,x) + 8*Ot(Ny-1,x))/(5+8+9+8);

		//handle last row separately
        O(Ny-1,x) = (5*Ot(Ny-3,x) + 8*Ot(Ny-2,x) + 9*Ot(Ny-1,x))/(5+8+9);
	}
    return O;
}

SimpleMatrix<double> mean2D(const SubImage& I, int sz){
    int Ny = I.nRows();
    int Nx = I.nCols();

    SimpleMatrix<double> Ot(Ny,Nx);

    //proc y
    for(size_t y=0;y<Ny;++y){
        for(int x=0;x<Nx;++x){
            Ot(y,x) = 0;
            int cnt = 0;
            for(int n = -sz/2;n<=sz/2;++n){
                int xi = x+n;
                if(xi>=0&&xi<Nx){
                    //mexPrintf("xi: %d\n",xi);
                    cnt++;
                    Ot(y,x)+=I(y,xi);
                }
            }
            Ot(y,x)/=cnt;
        }
    }

    SimpleMatrix<double> O(Ny,Nx);
    //proc x
    for(size_t x=0;x<Nx;++x){
        for(int y=0;y<Ny;++y){
            O(y,x) = 0;
            int cnt = 0;
            for(int n = -sz/2;n<=sz/2;++n){
                int yi = y+n;
                if(yi>=0&&yi<Ny){
                    cnt++;
                    O(y,x)+=Ot(yi,x);
                }
            }
            O(y,x)/=cnt;
        }
    }
    return O;
}

void recursive_fill(const bool* src, bool* dst,int r, int c, size_t H,size_t W, bool EightConnect){
    if(r<0){return;}
    if(c<0){return;}
    if(r>=H){return;}
    if(c>=W){return;}

    size_t i = r+c*H;
    if(dst[i]) {return;} //already set in dst
    if(!src[i]) {return;} //not present in src
    dst[i] = true;

    recursive_fill(src,dst,r+1,c,H,W,EightConnect);//try north
    recursive_fill(src,dst,r-1,c,H,W,EightConnect);//try south
    recursive_fill(src,dst,r,c+1,H,W,EightConnect);//try east
    recursive_fill(src,dst,r,c-1,H,W,EightConnect);//try west

    if(EightConnect){
        recursive_fill(src,dst,r+1,c+1,H,W,EightConnect);//try northeast
        recursive_fill(src,dst,r-1,c+1,H,W,EightConnect);//try southeast
        recursive_fill(src,dst,r+1,c-1,H,W,EightConnect);//try northwest
        recursive_fill(src,dst,r-1,c-1,H,W,EightConnect);//try southwest
    }

}

std::vector<double> sort(const SubImage& img){
    using namespace std;
    vector<double> srtI;
	srtI.reserve(img.nCols()*img.nRows());

    for(size_t x=0;x<img.nCols();x++){
        for(size_t y=0;y<img.nRows();y++){
            srtI.push_back(img(y,x));
        }
    }
    std::sort(srtI.begin(),srtI.end());
    return srtI;
}

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    //validate Inputs
    if(nrhs<1){
        mexErrMsgTxt("Not enough inputs");
    }

    if(mxIsComplex(prhs[0])){
        throw("Input Image must not be complex.");
    }

    if(mxGetNumberOfDimensions(prhs[0])!=2){
        throw("Input Image must be a matrix, i.e. ndim(Image)==2");
    }

    size_t HEIGHT = mxGetM(prhs[0]);
    size_t WIDTH = mxGetN(prhs[0]);

	// SECOND INPUT: WIND
    mexMatrixT<double> WIND; //[x,y,W,H]
    if(nrhs>1){
        WIND = prhs[1];
    }

	if (WIND.IsEmpty()) {
		WIND.resize(1, 4);
		WIND[0] = 0; //x
		WIND[1] = 0;//y
		WIND[2] = WIDTH; //W
		WIND[3] = HEIGHT; //H
	}

    if(WIND.nCols()!=4){
        mexErrMsgTxt("WIND must be n x 4 matrix");
    }
#ifdef DEBUG_PLOT
	mexPrintf("WIND:\n");
	WIND.disp();
#endif

    // THIRD INPUT: size limits
    mexMatrixT<double> SizeLim;
    if(nrhs<3){
        SizeLim.resize(WIND.nRows(),1);
        double sz = mxGetNaN();
        for(size_t n=0;n<SizeLim.numel();n++){
            SizeLim[n] = sz;
        }
    }else{
        SizeLim = prhs[2];
        if(SizeLim.numel()==1){
            double sz = SizeLim[0];
            SizeLim.resize(WIND.nRows(),1);
            for(size_t n=0;n<SizeLim.numel();n++){
                SizeLim[n] = sz;
            }
        }
    }

    if(SizeLim.numel()!=WIND.nRows()){
        mexErrMsgTxt("SizeLim must be scalar or have same numel as rows in WIND");
    }
#ifdef DEBUG_PLOT
	mexPrintf("SizeLim:\n");
	SizeLim.disp();
#endif

	// FOURTH INPUT: LimFrac
    double LimFrac;
    if(nrhs<4){// no LimFrac defined
        LimFrac = DEFAULT_LIM_FRAC;
    }else{
        LimFrac = mxGetScalar(prhs[3]);
    }
#ifdef DEBUG_PLOT
	mexPrintf("LimFrac:\n");
	mexPrintf("\t%g\n", LimFrac);
#endif

    mexMatrixT<double> X;
    mexMatrixT<double> Y;
    X.resize(WIND.nRows(),1);
    Y.resize(WIND.nRows(),1);

	mexMatrixT<double> mxX;
	mexMatrixT<double> mxY;
	mexMatrixT<double> mnX;
	mexMatrixT<double> mnY;

	if(nlhs>2){
		mxX.resize(WIND.nRows(),1);
		mxY.resize(WIND.nRows(),1);
		mnX.resize(WIND.nRows(),1);
		mnY.resize(WIND.nRows(),1);
	}

    //loop over windows and find positions
    for(size_t w=0;w<WIND.nRows();w++){
        X[w] = mxGetNaN(); //default value is nan
        Y[w] = mxGetNaN(); //default value is nan

		if(nlhs>2){
			mxX[w] = mxGetNaN();
			mxY[w] = mxGetNaN();
			mnX[w] = mxGetNaN();
			mnY[w] = mxGetNaN();
		}

        size_t X0 = fmax(0,fmin(WIDTH-1,WIND(w,0)));
        size_t Y0 = fmax(0,fmin(HEIGHT-1,WIND(w,1)));
        size_t W = WIND(w,2);
        size_t H = WIND(w,3);
        size_t X1 = fmax(0,fmin(WIDTH-1,X0+W-1));
        size_t Y1 = fmax(0,fmin(HEIGHT-1,Y0+H-1));
        W = X1-X0+1;
        H = Y1-Y0+1;


        if(X0>=X1||Y0>=Y1){ //zero width/height probably because window is out of bounds
            continue; //just process next window
        }

        // Get Sub Image using window bounds
        SubImage windImg= mx2SubImage(prhs[0],Y0,X0,H,W);
		
#ifdef DEBUG_PLOT
		SimpleMatrix<double> wI(H,W);
		for(size_t x=0;x<W;++x){
			for(size_t y=0;y<H;++y){
				wI(y,x) = windImg(y,x);
			}
		}
		mexEvalString("figure");
		imagesc(wI);
		mexEvalString("title('windImg');");
#endif

        // compute moving mean of sub Image
#ifdef GI_P5X5
        SimpleMatrix<double> gI = parab5x5(windImg);
#elif GI_M5X5
		SimpleMatrix<double> gI = mean5x5(windImg);
#elif GI_M3X3
		SimpleMatrix<double> gI = mean3x3(windImg);
#endif

#ifdef DEBUG_PLOT
#ifdef USE_GI
        mexEvalString("figure");
        imagesc(gI);
		mexEvalString("title('gI');");
#endif
#endif

        //find max, min of blurred image, find average of original image
#ifdef USE_GI
		double maxVal = gI(0,0);
#else
		double maxVal = windImg(0, 0);
#endif
        int maxIndx = 0;
        int maxIndy = 0;

#ifdef USE_GI
		double minVal = gI(0, 0);
#else
		double minVal = windImg(0, 0);
#endif
        int minIndx = 0;
        int minIndy = 0;

        double meanVal = 0; //also find mean of window, used in com calculation

        for(int x=0;x<W;x++){
            for(int y=0;y<H;y++){
#ifdef USE_GI
				if (gI(y, x)>maxVal) {
					maxVal = gI(y, x);
#else
				if (windImg(y, x)>maxVal) {
					maxVal = windImg(y, x);
#endif
                    maxIndx = x;
                    maxIndy = y;
                }

#ifdef USE_GI
				if (gI(y, x)<minVal) {
					minVal = gI(y,x);
#else
				if (windImg(y, x)<minVal) {
					minVal = windImg(y, x);
#endif
                    minIndx = x;
                    minIndy = y;
                }
                meanVal += windImg(y,x);
            }
        }
        meanVal/=(H*W);

		if(nlhs>2){
			mxX[w] = double(maxIndx) + double(X0) +1;
			mxY[w] = double(maxIndy) + double(Y0)+1;
			mnX[w] = double(minIndx) + double(X0)+1;
			mnY[w] = double(minIndy) + double(Y0)+1;
		}


        // Reduce window to SizeLim around max and min pixels
        int sX0;
        int sX1;
        int sY0;
        int sY1;
        int sW;
        int sH;
        if(mxIsNaN(SizeLim[w])){
            sX0 = 0;
            sX1 = W-1;
            sY0 = 0;
            sY1 = H-1;
            sW = W;
            sH = H;
        }
        else{
            sX0 = vmax((int)0,vmin(minIndx,maxIndx)-(int)SizeLim[w]);
            sX1 = vmin((int)W-1,vmax(minIndx,maxIndx)+(int)SizeLim[w]);
            sY0 = vmax((int)0,vmin(minIndy,maxIndy)-(int)SizeLim[w]);
            sY1 = vmin((int)H-1,vmax(minIndy,maxIndy)+(int)SizeLim[w]);
            sW = sX1-sX0+1;
            sH = sY1-sY0+1;
        }
#ifdef DEBUG_PLOT
		mexPrintf("SizeLim[w]=%d\n", (int)SizeLim[w]);
		mexPrintf("minIndx=%d, maxIndx=%d\n", minIndx, maxIndx);
		mexPrintf("minIndy=%d, maxIndy=%d\n", minIndy, maxIndy);
		mexPrintf("sX0=%d, sX1=%d, sY0=%d, sY1=%d, sW=%d, sH=%d\n", sX0, sX1, sY0, sY1, sW, sH);
#endif

        size_t sMaxIndy = maxIndy - sY0;
        size_t sMinIndy = minIndy - sY0;
        size_t sMaxIndx = maxIndx - sX0;
        size_t sMinIndx = minIndx - sX0;

        //define new sub image
        SubImage subI( windImg.ElementPtr(sY0,sX0),windImg.ClassID(),sH,sW,windImg.stride());
#ifdef USE_GI
		SubImage sgI( gI.ElementPtr(sY0,sX0),mxDOUBLE_CLASS,sH,sW,gI.stride());
#endif

        //Sort Data to figue out threshold limits
        //std::vector<double> srtI = sort(subI);

        //double lowerThresh = srtI[ceil(LimFrac*srtI.size())];
		//double upperThresh = srtI[floor((1-LimFrac)*srtI.size())];
#ifdef USE_GI
		double rangeFrac = LimFrac*(sgI(sMaxIndy,sMaxIndx)-sgI(sMinIndy,sMinIndx));
		double lowerThresh = sgI(sMinIndy, sMinIndx) + rangeFrac;
		double upperThresh = sgI(sMaxIndy, sMaxIndx) - rangeFrac;
#else
		double rangeFrac = LimFrac*(subI(sMaxIndy, sMaxIndx) - subI(sMinIndy, sMinIndx));
		double lowerThresh = subI(sMinIndy, sMinIndx) + rangeFrac;
		double upperThresh = subI(sMaxIndy, sMaxIndx) - rangeFrac;
#endif

        //define upper and lower masks
        SimpleMatrix<bool> uM(sH,sW,true);
        SimpleMatrix<bool> lM(sH,sW,true);

        for(size_t x=0;x<sW;x++){
            for(size_t y=0;y<sH;y++){
#ifdef USE_GI
				if(sgI(y,x)>=upperThresh){
#else
				if (subI(y, x) >= upperThresh) {
#endif
                    uM(y,x) = true;
                }
#ifdef USE_GI
				if (sgI(y, x) <= lowerThresh) {
#else
				if (subI(y, x) <= lowerThresh){
#endif
                    lM(y,x) = true;
                }
            }
        }

        //perform flood fill to get contiguous regions
        SimpleMatrix<bool> uM2(sH,sW,true);
        SimpleMatrix<bool> lM2(sH,sW,true);

        recursive_fill(uM.data(),uM2.data(),sMaxIndy,sMaxIndx,sH,sW,false);
        recursive_fill(lM.data(),lM2.data(),sMinIndy,sMinIndx,sH,sW,false);

#ifdef DEBUG_PLOT

        SimpleMatrix<double> fills(sH,sW);
        for(size_t x=0;x<sW;x++){
            for(size_t y=0;y<sH;y++){
                fills(y,x) = double(uM2(y,x)) - double(lM2(y,x));
            }
        }
        mexEvalString("figure");
        imagesc(fills);
        mexEvalString("title('thresh after recursive fill')");

        mexEvalString("hold on");
        mxArray* mexXY[3];
        mexXY[0] = mxCreateDoubleScalar(sMaxIndx+1);
        mexXY[1] = mxCreateDoubleScalar(sMaxIndy+1);
        mexXY[2] = mxCreateString("sr");
        mxArray* err = mexCallMATLABWithTrap(0,nullptr,3,mexXY,"plot");

        mxDestroyArray(mexXY[0]);
        mxDestroyArray(mexXY[1]);
        mxDestroyArray(mexXY[2]);


        mexXY[0] = mxCreateDoubleScalar(sMinIndx+1);
        mexXY[1] = mxCreateDoubleScalar(sMinIndy+1);
        mexXY[2] = mxCreateString("sc");
        err = mexCallMATLABWithTrap(0,nullptr,3,mexXY,"plot");

        if(err){
            mexPrintf("Error occured while trying to plot on image\n");
            //mexEvalString("pause()");
        }
        mxDestroyArray(mexXY[0]);
        mxDestroyArray(mexXY[1]);
        mxDestroyArray(mexXY[2]);

#endif //DEBUG_PLOT

        // Calc COM on upper and lower
        double upperAcc = 0;
        double upperXacc = 0;
        double upperYacc = 0;
        double lowerXacc = 0;
        double lowerYacc = 0;
        double lowerAcc = 0;
        double v;
        for(size_t x=0;x<sW;x++){
            for(size_t y=0;y<sH;y++){
                if(uM2(y,x)){
                    //double v =fabs(subI(y,x)-meanVal); //upper always positive
                    v = subI(y,x)-meanVal;
                    upperAcc+=v;
                    upperXacc+=double(x)*v;
                    upperYacc+=double(y)*v;
                }
                if(lM2(y,x)){
                    //double v =fabs(subI(y,x)-meanVal); //lower always negative
                    v = meanVal - subI(y,x);
                    lowerAcc+=v;
                    lowerXacc+=double(x)*v;
                    lowerYacc+=double(y)*v;
                }
            }
        }
#ifdef DEBUG_PLOT
        mexXY[0] = mxCreateDoubleScalar(upperXacc/upperAcc + 1);
        mexXY[1] = mxCreateDoubleScalar(upperYacc/upperAcc + 1);
        mexXY[2] = mxCreateString("+r");
        err = mexCallMATLABWithTrap(0,nullptr,3,mexXY,"plot");

        if(err){
            mexPrintf("Error occured while trying to plot on image\n");
            //mexEvalString("pause()");
        }
        mxDestroyArray(mexXY[0]);
        mxDestroyArray(mexXY[1]);
        mxDestroyArray(mexXY[2]);

        mexXY[0] = mxCreateDoubleScalar(lowerXacc/lowerAcc + 1);
        mexXY[1] = mxCreateDoubleScalar(lowerYacc/lowerAcc + 1);
        mexXY[2] = mxCreateString("+c");
        err = mexCallMATLABWithTrap(0,nullptr,3,mexXY,"plot");

        if(err){
            mexPrintf("Error occured while trying to plot on image\n");
            //mexEvalString("pause()");
        }
        mxDestroyArray(mexXY[0]);
        mxDestroyArray(mexXY[1]);
        mxDestroyArray(mexXY[2]);
        
#endif //DEBUG_PLOT

        if( (upperAcc==0&&upperXacc==0) || (lowerAcc==0&&lowerXacc==0)){
            continue; //value will be NaN
        }
        X[w] = double(X0)+double(sX0)+ ( upperXacc/upperAcc + lowerXacc/lowerAcc )/2.0+1.0; //add 1 for 1-based indexing (MATLAB)
        Y[w] = double(Y0)+double(sY0)+ ( upperYacc/upperAcc + lowerYacc/lowerAcc )/2.0+1.0; //add 1 for 1-based indexing (MATLAB)



    }
    plhs[0] = X;
    if(nlhs>1){
        plhs[1] = Y;
    }
	if(nlhs>2){
		plhs[2] = mxX;
	}
	if(nlhs>3){
		plhs[3] = mxY;
	}
	if(nlhs>4){
		plhs[4] = mnX;
	}
	if(nlhs>5){
		plhs[5] = mnY;
	}
}
