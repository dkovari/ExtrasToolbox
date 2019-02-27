#pragma once

#include <mex.h>
#include <extras/string_extras.hpp>
#include "radialcenter.hpp"

#include <extras/Array.hpp>
#include <extras/cmex/NumericArray.hpp>
#include <extras/cmex/mxparamparse.hpp>

namespace extras{namespace ParticleTracking{

    template<class OutContainerClass> //C must be and ArrayBase derived class
    std::vector<OutContainerClass> radialcenter(const mxArray* pI,
                                const extras::ArrayBase<double>& WIND,
                                const extras::ArrayBase<double>& GP,
                                const rcdefs::RCparams& params)
    {
        switch (mxGetClassID(pI)) { //handle different image types seperatelys
    	case mxDOUBLE_CLASS:
    		return radialcenter<OutContainerClass,double>(cmex::NumericArray<double>(pI), WIND, GP, params);
    	case mxSINGLE_CLASS:
    		return radialcenter<OutContainerClass,float_t>(cmex::NumericArray<float>(pI), WIND, GP, params);
    	case mxINT8_CLASS:
    		return radialcenter<OutContainerClass,int8_t>(cmex::NumericArray<int8_t>(pI), WIND, GP, params);
    	case mxUINT8_CLASS:
    		return radialcenter<OutContainerClass,uint8_t>(cmex::NumericArray<uint8_t>(pI), WIND, GP, params);
    	case mxINT16_CLASS:
    		return radialcenter<OutContainerClass,int16_t>(cmex::NumericArray<int16_t>(pI), WIND, GP, params);
    	case mxUINT16_CLASS:
    		return radialcenter<OutContainerClass,uint16_t>(cmex::NumericArray<uint16_t>(pI), WIND, GP, params);
    	case mxINT32_CLASS:
    		return radialcenter<OutContainerClass,int32_t>(cmex::NumericArray<int32_t>(pI), WIND, GP, params);
    	case mxUINT32_CLASS:
    		return radialcenter<OutContainerClass,uint32_t>(cmex::NumericArray<uint32_t>(pI), WIND, GP, params);
    	case mxINT64_CLASS:
    		return radialcenter<OutContainerClass,int64_t>(cmex::NumericArray<int64_t>(pI), WIND, GP, params);
    	case mxUINT64_CLASS:
    		return radialcenter<OutContainerClass,uint64_t>(cmex::NumericArray<uint64_t>(pI), WIND, GP, params);
    	default:
    		throw(std::runtime_error("radialcenter: Only numeric image types allowed"));
    	}
    }

    /// Wrapper for radialcenter, accepting the standard arguments for a mexFunction
    /*
    % [x,y,varXY,d2] = radialcenter(I,WIND,GP)
    %                = radialcenter(__,name,value);
    %
    % Estimate the center of radial symmetry of an image
    %
    % Input:
    %   I: the image to process
    %   WIND: [N x 4] specifying windows [x,y,w,h], default is entire image
    %   GP (default=5): optional exponent factor to use for magnitude weighting
    %       GP must be either scalar, or numel(GP)==size(WIND,1)
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
    %   'RadiusFilter',val or [v1,v2,...vN]
    %   'XYc',[X,Y] : particle center estimates
    %   'COMmethod',method
    %       method='meanABS' : use COM on |I-mean(I)| to estimate center for radius filter
    %       method='normal': use COM on unmodified I to estimate center
    %       method='gradmag': use magnitude of image gradient to find COM
    %   'DistanceFactor',value
    %       Rate to use in logistic function defining the filter window around xc,yc
    %       Default value is Inf, which indicates the Hat-function:
    %           W=double(r<Radius) is used instead of a logistic function.
    %       If RadiusFilter==0 DistanceFactor is the exponent of the inverse
    %       distance function use to weight the pixels:
    %           w = w/r^DF
    %       where r is the distance of a pixel from the extimated com or the
    %       specified XYc(n,:) coordinate
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

    	}


    	cmex::NumericArray<double> GP;

    	if (nrhs > 2 && ParamIndex>1 && !mxIsChar(prhs[2])) {
    		GP = prhs[2];
    		ParamIndex = 3;
    	}

    	rcdefs::RCparams params;

    	/// Parse value pair inputs
    	cmex::MxInputParser Parser(false); //create non-case sensitive input parser
    	Parser.AddParameter("RadiusFilter"); //create empty parameter
    	Parser.AddParameter("XYc");
    	Parser.AddParameter("COMmethod", "meanABS");
    	Parser.AddParameter("DistanceFactor", INFINITY);

    	if (ParamIndex < nrhs) {
    		int res = Parser.Parse(nrhs - ParamIndex, &prhs[ParamIndex]);
    		if (res != 0) {
    			throw(std::runtime_error("could not parse input parameters"));
    		}

    		params.RadiusFilter = std::make_shared<cmex::NumericArray<double>>(Parser("RadiusFilter"));
    		params.XYc = std::make_shared<cmex::NumericArray<double>>(Parser("XYc"));
    		//shift from 1-indexing
    		(*params.XYc.get())-=1;

    		params.DistanceFactor = fabs(mxGetScalar(Parser("DistanceFactor")));
    		//mexPrintf("Using params.DistanceFactor=%f\n",params.DistanceFactor);

    		std::string COMmeth = cmex::getstring(Parser("COMmethod"));

    		//validate COMmethod
    		COMmeth = tolower(COMmeth);

    		if (COMmeth.compare("meanabs") == 0) {
    			params.COMmethod = rcdefs::MEAN_ABS;
    		}
    		else if (COMmeth.compare("normal") == 0) {
    			params.COMmethod = rcdefs::NORMAL;
    		}
    		else if (COMmeth.compare("gradmag") == 0) {
    			params.COMmethod = rcdefs::GRAD_MAG;
    		}
    		else {
    			throw(std::runtime_error("COMmethod invalid"));
    		}
    	}

    	//mexPrintf("About to run radial center...\n");
    	try {
    		auto out = radialcenter<cmex::NumericArray<double>>(prhs[0], WIND, GP, params);

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
    	}
    	catch (std::exception& e) {
    		mexErrMsgTxt(e.what());
    	}
    }
}}