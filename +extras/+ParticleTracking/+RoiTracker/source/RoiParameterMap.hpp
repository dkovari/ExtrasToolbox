/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once;

#include <extras/async/ParamProcessor.hpp>
#include <extras/cmex/MxObject.hpp>
#include <extras/Array.hpp>
#include <extras/cmex/MxStruct.hpp>
#include <extras/string_extras.hpp>
#include <extras/cmex/mexextras.hpp>
#include <extras/cmex/NumericArray.hpp>

namespace extras {namespace ParticleTracking {

	class RoiParameterMap : public extras::cmex::ParameterMxMap {
	protected:
		std::shared_ptr<extras::ArrayBase<double>> WIND = std::make_shared<extras::Array<double>>(0);
		std::shared_ptr<extras::ArrayBase<double>> XYc = std::make_shared<extras::Array<double>>(0);
		std::shared_ptr<extras::ArrayBase<double>> GP = std::make_shared<extras::Array<double>>(0);
		std::shared_ptr<extras::ArrayBase<double>> RadiusFilter = std::make_shared<extras::Array<double>>(0);

		std::vector<std::string> _valid_xyMethods = { "radialcenter","barycenter" };
		std::vector<std::string> _valid_COMmethod = { "meanabs","gradmag","normal" };

		//! converts value to formatted xyMethod, throws error if value is valid
		//! change behavior by redefining _valid_xyMethods at construction
		cmex::MxObject valid_xyMethod(const cmex::MxObject& value) {
			if (!value.ischar()) {
				throw("xyMethod must be a char specifying valid ");
			}
			std::string s_val = cmex::getstring(value);
			if (!ismember(s_val, _valid_xyMethods, false)) {
				throw(std::runtime_error(std::string("xyMethod is not valid. Revieved: ")+s_val));
			}
			return extras::tolower(s_val);
		}

		//! converts value to formatted COMmethod, throws error if value is valid
		//! change behavior by redefining _valid_xyMethods at construction
		cmex::MxObject valid_COMmethod(const cmex::MxObject& value) {
			if (!value.ischar()) {
				throw("COMmethod must be a char specifying valid ");
			}
			std::string s_val = cmex::getstring(value);
			if (!ismember(s_val, _valid_COMmethod, false)) {
				throw("COMmethod is not valid");
			}
			return extras::tolower(s_val);
		}

		cmex::MxObject valid_DistanceFactor(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("DistanceFactor must be numeric");
			}
			if (value.numel() != 1) {
				throw("DistanceFacor must be scalar numeric");
			}

			return value;
		}

		cmex::MxObject valid_LimFrac(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("LimFrac must be numeric");
			}
			if (value.numel() != 1) {
				throw("LimFrac must be scalar numeric");
			}

			return value;
		}

		//! validate roiList
		//! make sure Window field is present
		cmex::MxStruct valid_roiList(cmex::MxObject&& value) {

			mexPrintf("Inside RoiParameterMap::valid_roiList()\n");
			mexEvalString("pause(0.2)");

			if (!value.isstruct()) {
				throw("roiList must be a struct");
			}
			cmex::MxStruct rStruct(std::move(value));

			if (!rStruct.isfield("Window")) {
				throw("roiList must contain 'Window' field");
			}

			return rStruct;
		}

		void setRoiList(cmex::MxObject&& value) {
			mexPrintf("Inside RoiParameterMap::setRoiList()\n");
			mexEvalString("pause(0.2)");

			cmex::MxStruct rS = valid_roiList(std::move(value));

			mexPrintf("Inside RoiParameterMap::setRoiList()...past valid_roiList\n");
			mexEvalString("pause(0.2)");

			size_t len = rS.numel();

			mexPrintf("Inside RoiParameterMap::setRoiList()...past rS.numel()\n");
			mexEvalString("pause(0.2)");

			// convert Window to WIND
			std::shared_ptr<extras::ArrayBase<double>> newWIND = std::make_shared<extras::Array<double>>(len, 4);

			mexPrintf("Inside RoiParameterMap::setRoiList()...past  newWIND = std::make_shared<extras::Array<double>>(len, 4)\n");
			mexEvalString("pause(0.2)");

			for (size_t n = 0; n < len; ++n) {
				
				mexPrintf("Inside RoiParameterMap::setRoiList()...top of WIND loon n=%d\n",n);
				mexEvalString("pause(0.2)");

				const mxArray* pWindow = rS(n, "Window");

				mexPrintf("Inside RoiParameterMap::setRoiList()...past rS(n, Window)\n" );
				mexEvalString("pause(0.2)");

				cmex::NumericArray<double> Window(pWindow);

				mexPrintf("Inside RoiParameterMap::setRoiList()...past cmex::NumericArray<double> Window(pWindow);\n");
				mexEvalString("pause(0.2)");

				double val = Window[0];

				mexPrintf("Inside RoiParameterMap::setRoiList()...past double val = Window[0];;\n");
				mexEvalString("pause(0.2)");

				// set window
				(*newWIND)(n, 0) = Window[0]; //X1


				(*newWIND)(n, 1) = Window[0] + Window[2] - 1; //X2=x+w-1
				(*newWIND)(n, 2) = Window[1]; //Y1
				(*newWIND)(n, 3) = Window[1] + Window[3] - 1; //Y2=y+h-1
			}

			mexPrintf("Inside RoiParameterMap::setRoiList()...past newWIND\n");
			mexEvalString("pause(0.2)");

			// set XYc
			std::shared_ptr<extras::ArrayBase<double>> newXYc = XYc;
			if (rS.isfield("XYc")) {
				newXYc = std::make_shared<extras::Array<double>>(len, 2);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "XYc");
					cmex::NumericArray<double> Arr(pMa);

					// set window
					(*newXYc)(n, 0) = Arr[0];
					(*newXYc)(n, 1) = Arr[1];
				}
			}

			mexPrintf("Inside RoiParameterMap::setRoiList()...past newXYc\n");
			mexEvalString("pause(0.2)");

			// set GP
			std::shared_ptr<extras::ArrayBase<double>> newGP = GP;
			if (rS.isfield("GP")) {
				newGP = std::make_shared<extras::Array<double>>(len, 1);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "GP");
					cmex::NumericArray<double> Arr(pMa);

					// set window
					(*newGP)(n, 0) = Arr[0];
				}
			}

			mexPrintf("Inside RoiParameterMap::setRoiList()...past newGP\n");
			mexEvalString("pause(0.2)");

			// set RadiusFilter
			std::shared_ptr<extras::ArrayBase<double>> newRadiusFilter = RadiusFilter;
			if (rS.isfield("RadiusFilter")) {
				newRadiusFilter = std::make_shared<extras::Array<double>>(len, 1);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "RadiusFilter");
					cmex::NumericArray<double> Arr(pMa);

					// set window
					(*newRadiusFilter)(n, 0) = Arr[0];
				}
			}

			mexPrintf("Inside RoiParameterMap::setRoiList()...past newGP\n");
			mexEvalString("pause(0.2)");

			/////////////
			// Made it here without errors, set the values

			extras::cmex::ParameterMxMap::operator[]("roiList").takeOwnership(rS);

			mexPrintf("Inside RoiParameterMap::setRoiList() ... About to set shared_ptrs\n");
			mexEvalString("pause(0.2)");

			WIND = newWIND;
			GP = newGP;
			XYc = newXYc;
			RadiusFilter = newRadiusFilter;

		}

		//! internal setField function
		void setFieldValue(const std::string& field, const mxArray* mxa) {

			mexPrintf("Inside RoiParameterMap::setFieldValue()\n");
			mexEvalString("pause(0.2)");

			if (strcmpi("xyMethod", field.c_str()) == 0) {
				extras::cmex::ParameterMxMap::operator[]("xyMethod").takeOwnership(valid_xyMethod(mxa));
			}
			else if (strcmpi("COMmethod", field.c_str()) == 0) {
				extras::cmex::ParameterMxMap::operator[]("COMmethod").takeOwnership(valid_COMmethod(mxa));
			}
			else if (strcmpi("DistanceFactor", field.c_str()) == 0) {
				extras::cmex::ParameterMxMap::operator[]("DistanceFactor").takeOwnership(valid_DistanceFactor(mxa));
			}
			else if (strcmpi("LimFrac", field.c_str()) == 0) {
				extras::cmex::ParameterMxMap::operator[]("LimFrac").takeOwnership(valid_LimFrac(mxa));
			}
			else if (strcmpi("roiList", field.c_str()) == 0) {
				// roiList requires special set
				mexPrintf("Inside RoiParameterMap::setFieldValue()...about to call setRoiList()\n");
				mexEvalString("pause(0.2)");
				setRoiList(mxa);
			}
			else {
				throw("Invalid parameter name");
			}
		}


	public:
		virtual ~RoiParameterMap() {};

		RoiParameterMap() :extras::cmex::ParameterMxMap(false) {
			/// Add MAP Defaults
			extras::cmex::ParameterMxMap::operator[]("xyMethod").takeOwnership(cmex::MxObject("radialcenter"));
			extras::cmex::ParameterMxMap::operator[]("COMmethod").takeOwnership(cmex::MxObject("meanABS"));
			extras::cmex::ParameterMxMap::operator[]("DistanceFactor").takeOwnership(mxCreateDoubleScalar(INFINITY));
			extras::cmex::ParameterMxMap::operator[]("LimFrac").takeOwnership(mxCreateDoubleScalar(0.2));

			//Create default, empty struct for roiList
			extras::cmex::ParameterMxMap::operator[]("roiList").takeOwnership(cmex::MxStruct(0, { "Window","UUID" }));
		}

		//! redefine (settable) operator[] to check for valid args
		virtual extras::cmex::persistentMxArray& operator[](const std::string& field) {
			// validate fieldname
			if (!isparameter(field)) {
				throw(std::runtime_error("RoiParameterMap::operator[]: field is not valid"));
			}
			return extras::cmex::ParameterMxMap::operator[](field);
		}

		virtual const extras::cmex::persistentMxArray& operator[](const std::string& field)const {
			return extras::cmex::ParameterMxMap::operator[](field);
		}

		//! refefine setParameters to validate the arguments
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {

			mexPrintf("Inside RoiParameterMap::setParameters()\n");
			mexEvalString("pause(0.2)");

			if (canSetFromStruct() && nrhs == 1) { // set from struct allowed
				if (!mxIsStruct(prhs[0])) {
					throw(std::runtime_error("ParameterMxMap::setParameters(): a single mxArray* was passed, but is was not a struct."));
				}

				if (!mxGetNumberOfElements(prhs[0]) != 1) {
					throw(std::runtime_error("ParameterMxMap::setParameters(): struct array must be a scalar struct (i.e. numel==1)"));
				}

				cmex::MxStruct thisStruct(prhs[0]);
				auto fnames = thisStruct.fieldnames();
				for (size_t n = 0; n < thisStruct.number_of_fields(); n++) {
					setFieldValue(fnames[n], thisStruct(0, fnames[n].c_str()));
				}

			}


			if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParameterMxMap::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}

			// loop over args and set parameters
			for (size_t n = 0; n < nrhs - 1; n += 2) {
				setFieldValue(extras::cmex::getstring(prhs[n]), prhs[n + 1]);
			}

		}

		////////////////////////
		// Special Parameters

		std::shared_ptr<extras::ArrayBase<double>> get_WIND() const { return WIND; }
		std::shared_ptr<extras::ArrayBase<double>> get_XYc() const { return XYc; }
		std::shared_ptr<extras::ArrayBase<double>> get_GP() const { return GP; }
		std::shared_ptr<extras::ArrayBase<double>> get_RadiusFilter() const { return RadiusFilter; }
	};

}}