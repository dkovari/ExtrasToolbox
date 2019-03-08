/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <math.h>
#include <mex.h>
#include <algorithm>
#include "spline.h"

namespace extras{namespace ParticleTracking{


    //Create spline from mxArray struct storing MATLAB pp-spline
    int createspline(spline * pp, const mxArray* in){
        if(mxGetNumberOfElements(in)>1){
            return -1;
        }
        mxArray* pp_breaks_mx = mxGetField(in,0,"breaks");
        mxArray* pp_coefs_mx = mxGetField(in,0,"coefs");
        mxArray* pp_order_mx = mxGetField(in,0,"order");
        mxArray* pp_dim_mx = mxGetField(in,0,"dim");

        if( !pp_breaks_mx || !pp_coefs_mx || !pp_order_mx || !pp_dim_mx){
            //mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
            //    "pp must be a valid spline");
            return -1;
        }

        pp->dim = mxGetScalar(pp_dim_mx);
        pp->order = mxGetScalar(pp_order_mx);
        pp->nBreaks = mxGetNumberOfElements(pp_breaks_mx);
        pp->stride = (pp->nBreaks-1)*pp->dim;
        pp->breaks = (double*)mxGetData(pp_breaks_mx);
        pp->coefs = (double*)mxGetData(pp_coefs_mx);

        return 0;
    }

    /* Syntax:
    [z,varz] = splineroot(v,pp,dpp,TOL,minStep,maxItr)

    Simultaneously solve the problem:
    z_m = argmin( Sum_i(Sp_i(z)-V_mi))

    Input:
            v: Nx1 array of values to best fit
            pp: N-dimensional spline structure
            dpp: (optional) pre-calculated derivative of pp
            TOL: (default=0.001) Algorithm stop when (Sum_i(Sp_i(z)-V_mi)^2)<=TOL
            minStep:(default=20*eps) scalar
                    if the computed newton step is smaller than minStep the algorithm returns
    		maxItr: (default=10000) max num of iterations
    		minR2frac: (default=0.00001) min fractional difference in R2_N between successive iterations
    		MaxR2: (default=Inf) max initial R2, if R2 of val vs all knots in the spline is greater than MaxR2, the algorithm returns NaN
    */
    void splineroot_mex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
        if(nrhs<2){
            mexErrMsgIdAndTxt("MATLAB:splineroot:invalidNumInputs",
                "At least 2 inputs required.");
        }

        //Validate pp
        if(!mxIsStruct(prhs[1])){
            mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
                "pp must be a struct");
        }
        if(mxGetNumberOfElements(prhs[1])>1){
            mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
                "pp must be a single n-dimensional spline");
        }

        //construct spline pp
        spline pp;

        if(createspline(&pp, prhs[1])<0){
            mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
                "pp not a valid spline");
        }


        // Init dpp
        spline dpp;


        bool free_dpp = false;
        char * dpp_calc; //array noting which breaks of dpp have been calculated

        bool hasdpp = false;
        if(nrhs>2){ //user specified dpp
            if(!mxIsEmpty(prhs[2])){
                hasdpp = true;
            }
        }
        if(hasdpp){ //dpp specified, validate it
            if(createspline(&dpp, prhs[2])<0){
                mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
                    "dpp not a valid spline");
            }

            if(dpp.dim!=pp.dim){
                mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
                    "dpp dim does not match pp dim");
            }

            if(dpp.nBreaks!=pp.nBreaks){
                mexErrMsgIdAndTxt("MATLAB:splineroot:invalidInput",
                    "dpp does not have same number of breaks as pp");
            }

            //set calc flag for all dpp coefs
            dpp_calc = (char*)malloc((dpp.nBreaks-1)*sizeof(char));
            memset(dpp_calc,1,dpp.nBreaks-1);
        }
        else{ //no dpp, need to calc on the fly
            dpp.dim = pp.dim;
            dpp.order = pp.order-1;
            dpp.nBreaks = pp.nBreaks;
            dpp.breaks = pp.breaks ;
            dpp.stride = (dpp.nBreaks-1)*dpp.dim;
            dpp.coefs = (double*)malloc(dpp.stride*dpp.order*sizeof(double));
            free_dpp = true;

            //set calc flag for all dpp coefs to false
            dpp_calc = (char*)calloc((dpp.nBreaks-1),sizeof(char));

        }

        // Get TOL
        double TOL = 0.001;
        if(nrhs>3){ //specified TOL
            if(!mxIsEmpty(prhs[3])){
                TOL = mxGetScalar(prhs[3]);
            }
        }

        //minStep
        double minStep = 20*DBL_EPSILON;
        if(nrhs>4){
            if(!mxIsEmpty(prhs[4])){
                minStep = mxGetScalar(prhs[4]);
            }
        }

        //maxItr
        size_t maxItr = 10000;
        if(nrhs>5){ //spec maxItr
            if(!mxIsEmpty(prhs[5])){
                maxItr = (size_t)mxGetScalar(prhs[5]);
            }
        }

    	double minR2frac = 0;// 0.00001;
    	if (nrhs > 6) {
    		if (!mxIsEmpty(prhs[6])) {
    			minR2frac = mxGetScalar(prhs[6]);
    		}
    	}

    	double MaxR2 = INFINITY;
    	if (nrhs > 7) {
    		if (!mxIsEmpty(prhs[7])) {
    			MaxR2 = mxGetScalar(prhs[7]);
    		}
    	}

        // calculate
        double z;
        double varz;
    	size_t nItr;
    	double s;
    	double R2;
    	double dR2frac;
    	double initR2 = NAN;

        if(nlhs>1){ //need varz output
    		z = splineroot((double*)mxGetData(prhs[0]), pp, dpp, dpp_calc, &varz, pow(TOL, 2), maxItr, minStep, minR2frac, MaxR2, &nItr, &s, &R2, &dR2frac, &initR2);
        }else{ //dont calc varz
    		z = splineroot((double*)mxGetData(prhs[0]), pp, dpp, dpp_calc, nullptr, pow(TOL, 2), maxItr, minStep, minR2frac, MaxR2, &nItr, &s, &R2, &dR2frac, &initR2);
        }

        plhs[0] = mxCreateDoubleScalar(z);
        if(nlhs>1){
            plhs[1] = mxCreateDoubleScalar(varz);
        }

    	if (nlhs > 2) {
    		plhs[2] = mxCreateDoubleScalar(nItr);
    	}

    	if (nlhs > 2) {
    		plhs[3] = mxCreateDoubleScalar(s);
    	}

    	if (nlhs > 4) {
    		plhs[4] = mxCreateDoubleScalar(R2);
    	}

    	if (nlhs > 5) {
    		plhs[5] = mxCreateDoubleScalar(dR2frac);
    	}

    	if (nlhs > 6) {
    		plhs[6] = mxCreateDoubleScalar(initR2);
    	}

        //cleanup dpp arrays
        free(dpp_calc);
        if(free_dpp){
            free(dpp.coefs);
        }

    }
}}
