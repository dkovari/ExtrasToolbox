/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <mex.h>
#include <extras/string_extras.hpp>
#include "radialcenter.h"

#include <vector>
#include <extras/Array.hpp>
#include <extras/cmex/NumericArray.hpp>
#include <extras/cmex/mxparamparse.hpp>

namespace extras{namespace ParticleTracking{

    template<class OutContainerClass> //OutContainerClass must be and ArrayBase derived class with template type=double
    std::vector<OutContainerClass> radialcenter(const mxArray* pI,
                                const RadialcenterParameters& params = RadialcenterParameters())
    {
		//number of particles to find
		size_t nPart = std::max(std::max(size_t(1), params.nWIND),params.nXYc);

		//Setup Output variables
		//----------------------------
		std::vector<OutContainerClass> out;
		out.resize(4);

		auto& x = out[0];
		auto& y = out[1];
		auto& varXY = out[2];
		auto& RWR_N = out[3];
        //auto& oXYc = out[4];

        //oXYc.resize(nPart,2);

		// Resize output vars
		x.resize(nPart, 1);
		y.resize(nPart, 1);

		varXY.resize(nPart, 2);
		RWR_N.resize(nPart, 1);

		//Call radialcenter
		//---------------------
        switch (mxGetClassID(pI)) { //handle different image types seperatelys
    	case mxDOUBLE_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(double*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxSINGLE_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(float*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxINT8_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(int8_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxUINT8_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(uint8_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxINT16_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(int16_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxUINT16_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(uint16_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxINT32_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(int32_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxUINT32_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(uint32_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxINT64_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(int64_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	case mxUINT64_CLASS:
			radialcenter(x.getdata(), y.getdata(), varXY.getdata(), RWR_N.getdata(),
				(uint64_t*)mxGetData(pI), mxGetM(pI), mxGetN(pI),
				params);//,oXYc.getdata());
			break;
    	default:
    		throw(std::runtime_error("radialcenter: Only numeric image types allowed"));
    	}

		return out;
    }

    /// Wrapper for radialcenter, accepting the standard arguments for a mexFunction
    /*
    % [x,y,varXY,d2] = radialcenter(I,WIND)
    %                = radialcenter(__,name,value);
    %
    % Estimate the center of radial symmetry of an image
    %
    % Input:
    %   I: the image to process
    %   WIND: [N x 4] specifying windows [x,y,w,h], default is entire image
    %
    % Output:
    %   x,y: center positions
    %
    %   varXY: variance estimate of the fit
    %       varXY = [Vx,Vy], where Vx and Vy are the variances of each X and Y
    %
    %   d2: the square of the weighted residual, normalized by the effective number of pixels
    %       d2>>1 indicates poor localization. This roughly characterizes the
    %       distance between each gradient line and the determined center
    %       location.
    %
    %      In practice, d2 is a good metric for determing if an image has an
    %      apparent symmetric center, while varXY is useful for characterizing the
    %      precision of the fit
    %
    %
    % Name,Value Parameters:
    % -------------------------
    %   'RadiusCutoff',val or [v1,v2,...vN]: fringe size cutoff
	%	'CutoffFactor',val or [v1,v2,...vN]: size cutoff is applied by wieghting using a logistic function :1/(1 + exp(CutoffFactor*(r_guess - RadiusCutoff)));
	%		where r_guess is the estimated center of symmetry (either supplied via XYC or found by Image central moment, aka image "center of mass")
	%		default = INFINITY (i.e. top-hat function)
    %   'XYc',[X,Y] : particle center estimates
    %   'COMmethod',method
    %       method='meanABS' : use COM on |I-mean(I)| to estimate center for radius filter
    %       method='normal': use COM on unmodified I to estimate center
    %       method='gradmag': use magnitude of image gradient to find COM (defalut)
    %   'DistanceExponent',value or [v1,v2,...,vN]: distance scaling from center guess Wii *= 1/r_guess^(DistanceExponent)
    %	'GradientExponent',value or [v1,v2,...,vN]: gradient scaling from center guess Wii *= |GradI_i|^(DistanceExponent)
    */
    void radialcenter_mex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
    	if (nrhs<1) { //not enough inputs
    		mexErrMsgIdAndTxt("MATLAB:radialcenter:invalidNumInputs",
    			"At least one input required.");
    	}
        if (nlhs<1){ //nothing to do.
            return;
        }

    	cmex::NumericArray<double> WIND;
		bool found_wind = false;

    	int ParamIndex = 1;

    	if (nrhs > 1 && !mxIsChar(prhs[1])) {
    		WIND = prhs[1];
    		ParamIndex = 2;

            if(!WIND.isempty()){
                if(WIND.nCols()!=4){
                    throw(std::runtime_error("WIND must be [n x 4]"));
                }
                for(size_t n=0;n<WIND.nRows();++n){ //fix 1-index --> 0-index
                    WIND(n,0)-= 1;
                    WIND(n,1)-=1;
                }
            }
			found_wind = true;
    	}

		RadialcenterParameters params;
		cmex::MxInputParser Parser(false); //create non-case sensitive input parser
		Parser.AddParameter("RadiusCutoff",INFINITY); //default to no radius cutoff
		Parser.AddParameter("CutoffFactor", INFINITY); //default to top-hat function
		Parser.AddParameter("DistanceExponent", 1); //default to top-hat function
		Parser.AddParameter("GradientExponent", 5); //default to top-hat function
		Parser.AddParameter("XYc");
		Parser.AddParameter("COMmethod", "gradmag");
		if(!found_wind){
			Parser.AddParameter("Window");
		}

		//parse parameters
		if (ParamIndex < nrhs) {
			/// Parse value pair inputs
			int res = Parser.Parse(nrhs - ParamIndex, &prhs[ParamIndex]);
			if (res != 0) {
				throw(std::runtime_error("could not parse input parameters"));
			}
		}

		cmex::NumericArray<double> RadiusCutoff(Parser("RadiusCutoff"));
		params.RadiusCutoff = RadiusCutoff.getdata();
		params.nRadiusCutoff = RadiusCutoff.numel();

		cmex::NumericArray<double> CutoffFactor(Parser("CutoffFactor"));
		params.CutoffFactor = CutoffFactor.getdata();
		params.nCutoffFactor = CutoffFactor.numel();

		cmex::NumericArray<double> DistanceExponent(Parser("DistanceExponent"));
		params.DistanceExponent = DistanceExponent.getdata();
		params.nDistanceExponent = DistanceExponent.numel();

		cmex::NumericArray<double> GradientExponent(Parser("GradientExponent"));
		params.GradientExponent = GradientExponent.getdata();
		params.nGradientExponent = GradientExponent.numel();

		cmex::NumericArray<double> XYc(Parser("XYc"));
		XYc -= 1;//shift from 1-indexing
		params.XYc = XYc.getdata();
		params.nXYc = XYc.nRows();

		params.COMmethod = string2COMmethod(cmex::getstring(Parser("COMmethod")));

		if (!found_wind) {
			WIND = Parser("Window");
		}
		if (!WIND.isempty()) {
			assert_condition(WIND.nCols() == 4, "radialcenter(): WIND must have nCols==4");
		}
		params.WIND = WIND.getdata();
		params.nWIND = WIND.nRows();

    	//mexPrintf("About to run radial center...\n");
    	try {
    		auto out = radialcenter<cmex::NumericArray<double>>(prhs[0], params);

    		if (nlhs > 0) {
    			out[0]+=1;
    			plhs[0] = out[0];
    		}
    		if (nlhs > 1) {
    			out[1]+=1;
    			plhs[1] = out[1];
    		}
    		if (nlhs > 2) {
    			plhs[2] = out[2];
    		}
    		if (nlhs > 3) {
    			plhs[3] = out[3];
    		}
            /*if(nlhs>4){ //oXYc
                out[4]+=1; //shift back to 1-indexing
                plhs[4] = out[4];
            }*/
    	}
    	catch (std::exception& e) {
    		mexErrMsgTxt(e.what());
    	}
    }
}}
