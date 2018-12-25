#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <extras/Array.hpp>


#define MAXS 10000
#define MAXM 10000

namespace extras{namespace ParticleTracking{
    /* find contiguous areas */
    template<typename T>
    inline void metti_punto_sup(int pv,int qv,int jv,int kv,char *mask, T *im, unsigned char& sup,int & abv, long& sx, long& sy, long& s, int& sl, int* sp, int* sq, int*sj, int* sk) {
    if (mask[qv]!=2 && im[pv]>sup) {
      abv=im[pv]-sup;sx+=jv*abv;sy+=kv*abv;s+=abv;
      mask[qv]=2;
      sl++;
    //if (sl>=MAXS) throw("exit(3)");
    sp[sl]=pv; sq[sl]=qv; sj[sl]=jv; sk[sl]=kv;
    }
    }
    template<typename T>
    inline void metti_punto_inf(int pv,int qv,int jv,int kv,char *mask, T *im, unsigned char& inf,int & abv, long& sx, long& sy, long& s, int& sl, int* sp, int* sq, int*sj, int* sk) {
    if (mask[qv]!=1 && im[pv]<inf) {
      abv=inf-im[pv];sx+=jv*abv;sy+=kv*abv;s+=abv;
      mask[qv]=1;
      sl++;
    //if (sl>=MAXS) throw("exit(3)");
    sp[sl]=pv; sq[sl]=qv; sj[sl]=jv; sk[sl]=kv;
    }
    }

    /*
       Parameters:
       im            : the image
       start         : the index of the first pixel of the ROI
       width         : the width of the ROI
       height        : the height of the ROI
       linestride    : the line width of the whole image
       alpha         : the factor for defining the tresholds
       weight        : the weight of the light region vs. the dark region (%)
       x,y           : the resulting position

       REMEMBER: THIS FUNCTION PROBABLY ASSUMES ROW MAJOR IMAGE DATA!
    */
    template<typename T>
    void mass_center(
    		 T *im,
    		 int start,int width,int height,int linestride,
    		 double alpha,
    		 int weight,
    		 double *x,double *y
    		 ) {
      /* position and the value of extreme */
      int maxp,maxq,maxj,maxk,
        minp,minq,minj,mink;
      unsigned char maxv,minv;
      /* chopping top and bottom */
      unsigned char inf,sup;
      /* stack to the coverage of contiguous areas*/
      int * sp = new int[width*height];
      int *sq = new int[width*height];
      int*sj = new int[width*height];
      int* sk = new int[width*height];
      int sl;
      /* mask to cover adjacent areas */
      char *mask = new char[width*height];
      /* coordinate */
      int j,k,p,q;
      /* sums*/
      long s,sx,sy;
      /* final values x,y ligth/dark */
      double xl,yl,xd,yd;
      /* abbreviations*/
      int abv;

      //if (width*height>MAXM) throw("exit(2)");

      for (j=0;j<width*height;j++) mask[j]=0;

      /* minimum, maximum, average */
      maxp=minp=start;
      maxq=minq=0;
      maxj=minj=maxk=mink=0;
      maxv=minv=im[start];s=0;
      abv=linestride-width;
      for (k=0,p=start,q=0;k<height;k++,p+=abv) for (j=0;j<width;j++,p++,q++) {
        if (im[p]>maxv) {maxv=im[p];maxp=p;maxq=q;maxj=j;maxk=k;}
        if (im[p]<minv) {minv=im[p];minp=p;minq=q;minj=j;mink=k;}
        s+=im[p];
      }
      s/=width*height;

      /*  crop the top and bottom of image */
      sup=(unsigned char)((1.0-alpha)*s+alpha*maxv);
      inf=(unsigned char)((1.0-alpha)*s+alpha*minv);

      /* average over the adjacent upper */
      if (weight!=0) {
        sx=sy=s=0;sl=-1;
        metti_punto_sup(maxp,maxq,maxj,maxk,mask,im,sup,abv,sx,sy,s,sl,sp,sq,sj,sk);
        while (sl>=0) {
          p=sp[sl];q=sq[sl];j=sj[sl];k=sk[sl];sl--;
          if (j>0) metti_punto_sup(p-1,q-1,j-1,k,mask,im,sup,abv,sx,sy,s,sl,sp,sq,sj,sk);
          if (j<width-1) metti_punto_sup(p+1,q+1,j+1,k,mask,im,sup,abv,sx,sy,s,sl,sp,sq,sj,sk);
          if (k>0) metti_punto_sup(p-linestride,q-width,j,k-1,mask,im,sup,abv,sx,sy,s,sl,sp,sq,sj,sk);
          if (k<height-1) metti_punto_sup(p+linestride,q+width,j,k+1,mask,im,sup,abv,sx,sy,s,sl,sp,sq,sj,sk);
        }
        xl=(double)sx/s;
        yl=(double)sy/s;
      }
      else { xl=yl=0.0; }

      /* average over the adjacent lower */
      if (weight!=100) {
        sx=sy=s=0;sl=-1;
        metti_punto_inf(minp,minq,minj,mink,mask,im,inf,abv,sx,sy,s,sl,sp,sq,sj,sk);
        while (sl>=0) {
          p=sp[sl];q=sq[sl];j=sj[sl];k=sk[sl];sl--;
          if (j>0) metti_punto_inf(p-1,q-1,j-1,k,mask,im,inf,abv,sx,sy,s,sl,sp,sq,sj,sk);
          if (j<width-1) metti_punto_inf(p+1,q+1,j+1,k,mask,im,inf,abv,sx,sy,s,sl,sp,sq,sj,sk);
          if (k>0) metti_punto_inf(p-linestride,q-width,j,k-1,mask,im,inf,abv,sx,sy,s,sl,sp,sq,sj,sk);
          if (k<height-1) metti_punto_inf(p+linestride,q+width,j,k+1,mask,im,inf,abv,sx,sy,s,sl,sp,sq,sj,sk);
        }
        xd=(double)sx/s;
        yd=(double)sy/s;
      }
      else { xd=yd=0.0; }

      /* results */
      *x=(xl*weight+xd*(100-weight))/100;
      *y=(yl*weight+yd*(100-weight))/100;

      //delete arrays
      delete[] sp;
      delete[] sq;
      delete[] sj;
      delete[] sk;
      delete[] mask;
    }

    template<class OutContainerClass=extras::Array<double>, typename ImageType=double> //OutContainerClass should be a container with resize(size_t) and operator[size_t] methods
	std::vector<OutContainerClass> barycenter(const extras::ArrayBase<ImageType>& I, const extras::ArrayBase<double>& WIND, double LimFrac=0.2){
		using namespace std;
		if(I.ndims()!=2){
			throw(std::runtime_error("barycenter(): Input Image must be a matrix, i.e. ndim(Image)==2"));
		}

        // Create output array
        std::vector<OutContainerClass> out;
        out.resize(2);
        auto& X = out[0];
        auto& Y = out[1];

        size_t HEIGHT = I.nRows();
		size_t WIDTH = I.nCols();

        // If wind is empty, make temporary wind
        extras::Array<double> tmpWIND;
        const extras::ArrayBase<double>* pWIND = &WIND;
        if (WIND.isempty()) {
			tmpWIND.resize(1, 4);
			tmpWIND[0] = 0; //x
			tmpWIND[1] = 0;//y
			tmpWIND[2] = WIDTH; //W
			tmpWIND[3] = HEIGHT; //H
            pWIND = &tmpWIND;
		}

        if (pWIND->nCols() != 4) {
            throw(std::runtime_error("barycenter(): WIND must be n x 4 matrix"));
		}

        // Resize outputs
        X.resize(pWIND->nRows(),1);
        Y.resize(pWIND->nRows(),1);

        //loop over windows and find positions
		for (size_t w = 0; w<WIND.nRows(); w++) {
			X[w] = NAN; //default value is nan
			Y[w] = NAN; //default value is nan

            // Limit Windo to image extents
			size_t X0 = fmax(0, fmin(WIDTH - 1, WIND(w, 0)));
			size_t Y0 = fmax(0, fmin(HEIGHT - 1, WIND(w, 1)));
			size_t W = WIND(w, 2);
			size_t H = WIND(w, 3);
			size_t X1 = fmax(0, fmin(WIDTH - 1, X0 + W - 1));
			size_t Y1 = fmax(0, fmin(HEIGHT - 1, Y0 + H - 1));
			W = X1 - X0 + 1;
			H = Y1 - Y0 + 1;

			if (X0 >= X1 || Y0 >= Y1) { //zero width/height probably because window is out of bounds
				continue; //just process next window
			}

            const ImageType* windImg = &I(Y0,X0);

			double y;
			double x;

            mass_center(windImg,0,H,W,I.nRows(),LimFrac,50,&y,&x);

			X[w] = x+double(X0);
			Y[w] = y+double(Y0);
		}

        return out;
	}



}}
