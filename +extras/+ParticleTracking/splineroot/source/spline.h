/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once
#include <stdlib.h>
#include <math.h>
#include <float.h>

namespace extras{namespace ParticleTracking{
    //vector dot product
    // val = L.R
    // numel: number of elements in L and R
    double dotprod(const double * L,const double* R, size_t numel){
        double res = 0;
        for(size_t n=0;n<numel;n++){
            res += L[n]*R[n];
        }
        return res;
    }

    //vector subtraction: lhs= l-r
    // numel: number of elements in l and r
    void vecsub(double* lhs, const double* l, const double* r, size_t numel){
        for(size_t n=0;n<numel;n++){
            lhs[n] = l[n] - r[n];
        }
    }

    // Spline Structure
    // This structure holds pointers to the spline data (coefs and breaks)
    // as well as infomation about how the data is shaped.
    //
    // This is a simple c struct, so it doesn't manage the pointers
    // you are responsible for managing the memory asslciated with *coefs and *breaks
    struct spline{
        double* coefs; //column-major array specifying coefficients ( [stride x order])
        double* breaks; // array specifying break points (1-d array)
        size_t nBreaks;	//number of break points in *breaks
        size_t order; // order: the degree of polynomial used to define the spline
        size_t dim; //the dimensionality of the spline
        size_t stride; // stride of the coefficients matrix: stride=(nBreaks-1)*dim
    };

    //Calculate derivative of pp at brk, store result in dpp
    //Inputs:
    // brk: break point at which to calculte derivative
    // pp: spline struct used for calculating the derivative
    // dpp: spline struct used for storing calculated derivatives
    // dpp_calc: array with numel==nBreaks, each element specifies if the derivative in dpp has been calculated
    void ppder(size_t brk, spline pp, spline dpp, char* dpp_calc){
        if(brk>pp.nBreaks-1){ //brk is past end of nBreaks
            return;
        }
        if(dpp_calc[brk]){ //already calculated the derivative
            return;
        }

        for(size_t d=0;d<pp.dim;d++){
            for(size_t n=0;n<pp.order-1;n++){
                dpp.coefs[d+brk*dpp.dim+dpp.stride*n] = double(pp.order-1-n)*pp.coefs[d+brk*pp.dim+pp.stride*n];
            }
        }
        dpp_calc[brk] = true;
    }

    //Get Closest break to a point
    //Inputs:
    // x: location to find closest break
    // pp: the spline structure
    // brk=0: initial guess
    size_t closestbreak(double x, spline pp, size_t brk = 0){

        while(brk>0&&x<pp.breaks[brk]){
            brk--;
        }
        while( brk< pp.nBreaks-1 && x>= pp.breaks[brk+1] ){
            brk++;
        }
        return brk;
    }

    // evaluate spline at point
    // Inputs:
    // *val: pointer to array in which to store values
    // pp: the spline Structure
    // x: value to calc spline at
    // brk=0: initial guess of closest break point
    size_t ppval(double* val, spline pp, double x,size_t brk = 0){
        // find closest break
        brk = closestbreak(x,pp,brk);

        //calc values
        for(size_t d=0;d<pp.dim;d++){
            val[d] = pp.coefs[d+brk*pp.dim+pp.stride*(pp.order-1)];
            if(x>pp.breaks[brk]){
                for(size_t n=0;n<pp.order-1;n++){
                    val[d] += pp.coefs[d+brk*pp.dim+n*pp.stride]*pow(x-pp.breaks[brk],pp.order-1-n);
                }
            }
        }
        return brk;
    }

    //Find the break with the knot closest to the specified values
    //Inputs:
    //  *v: pointer to array of values to match
    //  pp: spline Structure
    // *finalR2=nullptr: pointer to variable in which R2 residual should be stored
    // *R2array=nullptr: pointer to array in which R2 values should be stored, R2array should have the same number of lements as pp.nBreaks
    size_t closestknot(const double * v, spline pp, double *finalR2 = nullptr) {
        size_t BestBreak;
        double R2 = INFINITY;

        for(size_t b=0;b<(pp.nBreaks-1);b++){
            double thisR2 = 0;
            for(size_t d=0;d<pp.dim;d++){
    			if (isfinite(v[d])) {
    				thisR2 += pow(v[d] - pp.coefs[d + b*pp.dim + pp.stride*(pp.order - 1)], 2);
    			}
            }
            if(thisR2<R2){
                R2 = thisR2;
                BestBreak = b;
            }
        }

        if(finalR2!=nullptr){
            *finalR2 = R2;
        }

        return BestBreak;
    }

    //Solve the multi-dimension function
    // V = pp(x)
    // Where pp is a spline function defined using the struct spline
    //Inputs:
    // *v: values to fit
    // pp: spline struct
    // dpp: spline struct for derivatives (must be same size as pp)
    // *dpp_calc: array (numel=num breaks) specifying if dpp has been calculated at each break point (1=calculated 0=not calculated)
    // *varz=nullptr: pointer to variable that will store estimated error variance (not calculated if *varz=nullptr)
    // TOL=0.001: max error tollerance
    // maxItr=10000: max number of iterations to run
    // minStep=20*DBL_EPSILON: min step size for each Newton step
    // min_dR2frac=0.00001: minimum fractional change in R2 between steps
    // MaxInitR2=INFINITY: maximum allowed R2 between *v and the spline's closest knot point
    double splineroot(const double* v,spline pp, spline dpp, char* dpp_calc, double* varz = nullptr,
    	double TOL=0.001, size_t maxItr=10000, double minStep = 20*DBL_EPSILON, double min_dR2frac = 0.00001, double MaxInitR2 = INFINITY,
    	size_t* nItr=nullptr, double* s_out=nullptr, double* R2_out=nullptr, double* dR2frac = nullptr, double* initR2=nullptr){

    	minStep = fabs(minStep);
    	// look for nan in v
    	size_t *badInd = (size_t*)malloc(pp.dim * sizeof(size_t));
    	size_t nBad = 0;
    	for (size_t n = 0; n < pp.dim; ++n) {
    		if (!isfinite(v[n])) {
    			badInd[nBad] = n;
    			++nBad;
    		}
    	}

    	if (nBad == pp.dim) {
    		if (nItr != nullptr) {
    			*nItr = 0;
    		}
    		if (s_out != nullptr) {
    			*s_out = NAN;
    		}

    		if (R2_out != nullptr) {
    			*R2_out = NAN;
    		}

    		if (dR2frac != nullptr) {
    			*dR2frac = NAN;
    		}
    		return NAN;
    	}

        //find closest knot
        double R2_N = INFINITY;
        double * r = (double*) malloc(pp.dim*sizeof(double));
        double * ppV = (double*) malloc(dpp.dim*sizeof(double));

        size_t brk = closestknot(v,pp,&R2_N); //find best fit among knot points

    	if (initR2 != nullptr) {

    		*initR2 = R2_N;
    	}

    	if (isfinite(MaxInitR2) && R2_N > MaxInitR2) {
    		//mexPrintf(">MaxR2\n");
    		if (nItr != nullptr) {
    			*nItr = 0;
    		}
    		if (s_out != nullptr) {
    			*s_out = NAN;
    		}

    		if (R2_out != nullptr) {
    			*R2_out = NAN;
    		}

    		if (dR2frac != nullptr) {
    			*dR2frac = NAN;
    		}
    		return NAN;
    	}



    	R2_N /= (pp.dim - 1);

        double z = pp.breaks[brk]; //z or current breakpoint

        ppval(ppV,pp,z,brk);//ppV = ppval(pp,z)
        vecsub(r,v,ppV,pp.dim);//r=v-ppV
    	for (size_t b = 0; b < nBad; ++b) {
    		r[badInd[b]] = 0;
    	}

    	//mexPrintf("z_start=%g\n", z);

        // Jacobean
        double * J = (double*) malloc(dpp.dim*sizeof(double));
        double J2; //|Jacobean|^2

        size_t itr = 0; //iteration count
        double s = INFINITY; //step_size
    	double lastR2; //last sq. residual
        do{
    		lastR2 = R2_N;
            // Jacobean & residual
            if(!dpp_calc[brk]){
                ppder(brk,pp,dpp,dpp_calc);
            }
            ppval(J,dpp,z,brk);

    		//set bad ind to zero
    		for (size_t b = 0; b < nBad; ++b) {
    			J[badInd[b]] = 0;
    		}


            J2 = dotprod(J,J,pp.dim);

            s = dotprod(J,r,pp.dim)/J2;



    		//mexPrintf("z=%g\n", z);



            //update brk
            //closestbreak(z,pp,brk); //now included with ppval, see below

            // update brk and calc new r
    		double lastz = z;
    		//int sub_div_s = 0;
    		do {
    			z = lastz + s; //update z
    			//check z;
    			if (z<pp.breaks[0] || z >= pp.breaks[pp.nBreaks - 1]) {
    				//mexPrintf("Out of spline range!\n");
    				z = NAN;
    				break;
    			}

    			//calc residual
    			brk = ppval(ppV, pp, z, brk); //ppV = pp(z)
    			vecsub(r, v, ppV, pp.dim);//r=v-ppV

    									  //set bad ind to zero
    			for (size_t b = 0; b < nBad; ++b) {
    				r[badInd[b]] = 0;
    			}

    			R2_N = dotprod(r, r, pp.dim) / double(pp.dim - 1 - nBad);

    			if (R2_N > lastR2) {
    				s /= 2;
    				//sub_div_s++;
    			}

    			//mexPrintf("in sub loop\n");
    		} while (R2_N > lastR2 && fabs(s)>minStep);
    		//mexPrintf("div s: %d\n", sub_div_s);

            itr++;

    		//mexPrintf("i: %d, R2:%g Tol: %g, s %e mns:%e dR2f:%g min_dR2f:%g\n", itr, R2_N, TOL, s, minStep, (R2_N - lastR2) / lastR2, min_dR2frac);

    	} while (isfinite(z) && itr < maxItr && R2_N >= TOL && fabs(s) > minStep);//&& (isinf(lastR2)||fabs(R2_N-lastR2)/lastR2>=min_dR2frac) );
        //mexPrintf("itr: %d, R2_N: %g, s:%g\n",itr,R2_N,s);

        //set var output
        if(varz!=nullptr){
            *varz = R2_N/J2;
        }
        //cleanup
        free(r);
        free(J);
        free(ppV);
    	free(badInd);

    	if (nItr != nullptr) {
    		*nItr = itr;
    	}
    	if (s_out != nullptr) {
    		*s_out = s;
    	}

    	if (R2_out != nullptr) {
    		*R2_out = R2_N;
    	}

    	if (dR2frac != nullptr) {
    		*dR2frac = fabs(R2_N - lastR2) / lastR2;
    	}

        return z;

    }
}}
