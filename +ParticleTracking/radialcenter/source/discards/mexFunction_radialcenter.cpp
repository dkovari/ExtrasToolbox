#include <mex.h>
#include <GenericArray.hpp>
#include <mexNumericArray.hpp>
#include <mxparamparse.hpp>
#include <mexextras.hpp>

#include "radialcenter.hpp"


std::string tolower(std::string && str) {
	for (auto & c : str) c = tolower(c);
	return str;
}
std::string tolower(const std::string & str) {
	std::string out(str);

	for (auto & c : out) c = tolower(c);
	return out;
}

std::string toupper(std::string && str) {
	for (auto & c : str) c = toupper(c);
	return str;
}
std::string toupper(const std::string &str) {
	std::string out(str);
	for (auto & c : out) c = toupper(c);
	return out;
}

std::vector<mex::NumericArray<double>> radcen(const mxArray* pI, mex::NumericArray<double>& WIND, mex::NumericArray<double>& GP, rcdefs::RCparams& params, size_t nlhs) {
	switch (mxGetClassID(pI)) { //handle different image types seperatelys
	case mxDOUBLE_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<double>(pI), WIND, GP, params, nlhs);
	case mxSINGLE_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<float>(pI), WIND, GP, params, nlhs);
	case mxINT8_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<int8_t>(pI), WIND, GP, params, nlhs);;
	case mxUINT8_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<uint8_t>(pI), WIND, GP, params, nlhs);;
	case mxINT16_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<int16_t>(pI), WIND, GP, params, nlhs);;
	case mxUINT16_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<uint16_t>(pI), WIND, GP, params, nlhs);;
	case mxINT32_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<int32_t>(pI), WIND, GP, params, nlhs);;
	case mxUINT32_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<uint32_t>(pI), WIND, GP, params, nlhs);;
	case mxINT64_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<int64_t>(pI), WIND, GP, params, nlhs);;
	case mxUINT64_CLASS:
		return radialcenter<double, mex::NumericArray<double>>(mex::NumericArray<uint64_t>(pI), WIND, GP, params, nlhs);;
	default:
		throw(std::runtime_error("radialcenter: Only numeric image types allowed"));
	}
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs<1) { //not enough inputs
		mexErrMsgIdAndTxt("MATLAB:radialcenter:invalidNumInputs",
			"At least one input required.");
	}

	// Input 1: I
	//mex::NumericArray<double> I(prhs[0]);

	mex::NumericArray<double> WIND;
	int ParamIndex = 1;

	if (nrhs > 1 && !mxIsChar(prhs[1])) {
		WIND = prhs[1];
		ParamIndex = 2;
	}


	mex::NumericArray<double> GP;

	if (nrhs > 2 && ParamIndex>1 && !mxIsChar(prhs[2])) {
		GP = prhs[2];
		ParamIndex = 3;
	}

	rcdefs::RCparams params;

	/// Parse value pair inputs
	mex::MxInputParser Parser(false); //create non-case sensitive input parser
	Parser.AddParameter("RadiusFilter"); //create empty parameter
	Parser.AddParameter("XYc");
	Parser.AddParameter("COMmethod", "meanABS");
	Parser.AddParameter("DistanceFactor", INFINITY);

	if (ParamIndex < nrhs) {
		int res = Parser.Parse(nrhs - ParamIndex, &prhs[ParamIndex]);
		if (res != 0) {
			throw(std::runtime_error("could not parse input parameters"));
		}

		params.RadiusFilter = std::make_shared<mex::NumericArray<double>>(Parser("RadiusFilter"));
		params.XYc = std::make_shared<mex::NumericArray<double>>(Parser("XYc"));
		//shift from 1-indexing
		(*params.XYc.get())-=1;

		params.DistanceFactor = fabs(mxGetScalar(Parser("DistanceFactor")));
		//mexPrintf("Using params.DistanceFactor=%f\n",params.DistanceFactor);

		std::string COMmeth = mex::getstring(Parser("COMmethod"));

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
		//std::vector<mex::NumericArray<double>> out = radialcenter<double,mex::NumericArray<double>>(I, WIND, GP, params, nlhs);
		std::vector<mex::NumericArray<double>> out = radcen(prhs[0], WIND, GP, params, nlhs);
		//set output
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
