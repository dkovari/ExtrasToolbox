
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAXS 10000
#define MAXM 10000



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

#ifdef DEBUG_IMAGE
  for (j=0;j<width;j++) for (k=0;k<height;k++) {
    if (mask[j+k*width]==1) im[start+j+k*linestride]=0;
    if (mask[j+k*width]==2) im[start+j+k*linestride]=255;
  }
  for (j=0;j<width;j++) im[start+j]=0;
  for (j=0;j<width;j++) im[start+j+(height-1)*linestride]=0;
  for (j=0;j<height;j++) im[start+j*linestride]=0;
  for (j=0;j<height;j++) im[start+j*linestride+width-1]=0;
#endif

  //delete arrays
  delete[] sp;
  delete[] sq;
  delete[] sj;
  delete[] sk;
  delete[] mask;
}


