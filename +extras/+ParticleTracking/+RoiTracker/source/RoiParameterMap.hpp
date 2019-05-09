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
#include <radialcenter/source/radialcenter.hpp>

namespace extras {namespace ParticleTracking {

	//! enum specifying xy localization algorithm
	enum XY_FUNCTION {
		BARYCENTER,
		RADIALCENTER
	};

	//! convert char array to XY_FUNCTION
	XY_FUNCTION xyFunction(const char* name) {
		if (strcmpi(name, "barycenter") == 0) {
			return BARYCENTER;
		}
		else if (strcmpi(name, "radialcenter") == 0) {
			return RADIALCENTER;
		}
		else {
			throw(std::runtime_error(std::string("xyMethod is not valid. Revieved: ") + std::string(name)));
		}
	}

	/** Specialized ParameterMxMap used by RoiTracker
	 *	This ParameterMap can be set by any name,value argument pairs (just like the standard ParameterMxMap)
	 *	However, it will always define the following fields
	 *		roiList
	 *		xyMethod
	 *		COMmethod
	 *		DistanceFactor
	 *		LimFrac
	 *
	 *	If you want to extend RoiTracker in a subclass you should consider 
	 *	redefining the virtual method setFieldValue()
	 */
	class RoiParameterMap : public extras::cmex::ParameterMxMap, public RadialcenterParameters_Shared {
	protected:

		double LimFrac = 0.2;
		XY_FUNCTION xyMethod = XY_FUNCTION::RADIALCENTER;

		double default_DistanceExponent = 0;
		double default_GradientExponent = 5;
		double default_RadiusCutoff = INFINITY;
		double default_CutoffFactor = INFINITY;

		void set_xyMethod(const cmex::MxObject& value) {
			if (!value.ischar()) {
				throw("xyMethod must be a char specifying valid ");
			}
			xyMethod = xyFunction(cmex::getstring(value).c_str());


			(*this)["xyMethod"] = value;
		}

		void set_COMmethod(const cmex::MxObject& value) {
			if (!value.ischar()) {
				throw("COMmethod must be a char specifying valid ");
			}
			COMmethod = string2COMmethod(cmex::getstring(value));

			(*this)["COMmethod"] = value;
		}

		void set_DistanceExponent(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("DistanceExponent must be numeric");
			}
			if (value.numel() != 1) {
				throw("DistanceExponent must be scalar numeric");
			}
			default_DistanceExponent = mxGetScalar(value);
			(*this)["DistanceExponent"] = value;
		}

		void set_GradientExponent(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("GradientExponent must be numeric");
			}
			if (value.numel() != 1) {
				throw("GradientExponent must be scalar numeric");
			}
			default_GradientExponent = mxGetScalar(value);
			(*this)["GradientExponent"] = value;
		}

		void set_RadiusCutoff(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("RadiusCutoff must be numeric");
			}
			if (value.numel() != 1) {
				throw("RadiusCutoff must be scalar numeric");
			}
			default_RadiusCutoff = mxGetScalar(value);
			(*this)["RadiusCutoff"] = value;
		}

		void set_CutoffFactor(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("CutoffFactor must be numeric");
			}
			if (value.numel() != 1) {
				throw("CutoffFactor must be scalar numeric");
			}
			default_CutoffFactor = mxGetScalar(value);
			(*this)["CutoffFactor"] = value;
		}

		void set_LimFrac(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("LimFrac must be numeric");
			}
			if (value.numel() != 1) {
				throw("LimFrac must be scalar numeric");
			}
			LimFrac = mxGetScalar(value);
			(*this)["LimFrac"] = value;
		}

		void set_roiList(const mxArray* mxa) {
			if(!mxIsStruct(mxa)) {
				throw("roiList must be a struct");
			}

			cmex::MxStruct rS(mxa);

			if (!rS.isfield("Window")) {
				throw("roiList must contain 'Window' field");
			}

			////////////
			// Convert to special format

			size_t len = rS.numel();

			// convert Window to WIND
			std::shared_ptr<extras::ArrayBase<double>> newWIND = std::make_shared<extras::Array<double>>(len, 4);
			for (size_t n = 0; n < len; ++n) {
				const mxArray* pWindow = rS(n, "Window");

				//validate windows
				if (!mxIsNumeric(pWindow) || mxGetNumberOfElements(pWindow) != 4) {
					throw(std::runtime_error("Window must be numeric 1x4 array"));
				}

				cmex::NumericArray<double> Window(pWindow);
	
				/* OLD WAY TO SET WIND
				(*newWIND)(n, 0) = Window[0]; //X1
				(*newWIND)(n, 1) = Window[0] + Window[2] - 1; //X2=x+w-1
				(*newWIND)(n, 2) = Window[1]; //Y1
				(*newWIND)(n, 3) = Window[1] + Window[3] - 1; //Y2=y+h-1
				*/

				/* NEW WAY: [x0,y0,w,h]*/
				(*newWIND)(n, 0) = Window[0];
				(*newWIND)(n, 1) = Window[1];
				(*newWIND)(n, 2) = Window[2];
				(*newWIND)(n, 3) = Window[3];

			}

			// set XYc
			std::shared_ptr<extras::ArrayBase<double>> newXYc = XYc;
			if (rS.isfield("XYc")) {
				newXYc = std::make_shared<extras::Array<double>>(len, 2);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "XYc");

					//validate xyc
					if (!mxIsNumeric(pMa) || mxGetNumberOfElements(pMa) != 2) {
						throw(std::runtime_error("XYc must be numeric 1x2 array"));
					}


					cmex::NumericArray<double> Arr(pMa);

					// set
					(*newXYc)(n, 0) = Arr[0];
					(*newXYc)(n, 1) = Arr[1];
				}
			}

			// DistanceExponent
			std::shared_ptr<extras::ArrayBase<double>> newDistanceExponent = DistanceExponent;
			if (rS.isfield("DistanceExponent")) {
				newDistanceExponent = std::make_shared<extras::Array<double>>(len,1);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "DistanceExponent");

					//validate DistanceExponent
					if (!mxIsNumeric(pMa) || mxGetNumberOfElements(pMa) != 1) {
						throw(std::runtime_error("DistanceExponent must be numeric 1x1 array"));
					}

					cmex::NumericArray<double> Arr(pMa);

					// set
					(*newDistanceExponent)(n, 0) = Arr[0];
				}
			}
			else {
				newDistanceExponent->operator=(default_DistanceExponent);
			}

			
			// GradientExponent
			std::shared_ptr<extras::ArrayBase<double>> newGradientExponent = GradientExponent;
			if (rS.isfield("GradientExponent")) {
				newDistanceExponent = std::make_shared<extras::Array<double>>(len, 1);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "GradientExponent");

					//validate GradientExponent
					if (!mxIsNumeric(pMa) || mxGetNumberOfElements(pMa) != 1) {
						throw(std::runtime_error("GradientExponent must be numeric 1x1 array"));
					}

					cmex::NumericArray<double> Arr(pMa);

					// set
					(*newGradientExponent)(n, 0) = Arr[0];
				}
			}
			else {
				newGradientExponent->operator=(default_GradientExponent);
			}

			// RadiusCutoff
			std::shared_ptr<extras::ArrayBase<double>> newRadiusCutoff = RadiusCutoff;
			if (rS.isfield("RadiusCutoff")) {
				newDistanceExponent = std::make_shared<extras::Array<double>>(len, 1);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "RadiusCutoff");

					//validate RadiusCutoff
					if (!mxIsNumeric(pMa) || mxGetNumberOfElements(pMa) != 1) {
						throw(std::runtime_error("RadiusCutoff must be numeric 1x1 array"));
					}

					cmex::NumericArray<double> Arr(pMa);

					// set
					(*newRadiusCutoff)(n, 0) = Arr[0];
				}
			}
			else {
				newRadiusCutoff->operator=(default_RadiusCutoff);
			}

			// CutoffFactor
			std::shared_ptr<extras::ArrayBase<double>> newCutoffFactor = CutoffFactor;
			if (rS.isfield("CutoffFactor")) {
				newDistanceExponent = std::make_shared<extras::Array<double>>(len, 1);
				for (size_t n = 0; n < len; ++n) {

					const mxArray* pMa = rS(n, "CutoffFactor");

					//validate RadiusCutoff
					if (!mxIsNumeric(pMa) || mxGetNumberOfElements(pMa) != 1) {
						throw(std::runtime_error("CutoffFactor must be numeric 1x1 array"));
					}

					cmex::NumericArray<double> Arr(pMa);

					// set
					(*newCutoffFactor)(n, 0) = Arr[0];
				}
			}
			else {
				newCutoffFactor->operator=(default_CutoffFactor);
			}


			/////////////////////////////////////////////////
			// Made it here without errors, set the values
			
			WIND = newWIND;

			RadiusCutoff = newRadiusCutoff;
			CutoffFactor = newCutoffFactor;
			XYc = newXYc;
			DistanceExponent = newDistanceExponent;
			GradientExponent = newGradientExponent;

			(*this)["roiList"] = mxa;
		}

		/** internal setField function
		* Redefine this method to add more functionality when user calls setParameters(...)
		* 
		*	The best approach is to intercept the field you want and pass the rest to this method
		* Example:
		*		yourMap::setFieldValue(...){
		*			...
		*			if(field=="YOUR_FIELD"){
		*				doSomething(...);
		*			}
		*			else{
		*				RoiParameterMap::setFieldValue(field,mxa);
		*			}
		*		}
		 */
		virtual void setFieldValue(const std::string& field, const mxArray* mxa) {

			if (strcmpi("xyMethod", field.c_str()) == 0) {
				set_xyMethod(mxa);
			}
			else if (strcmpi("COMmethod", field.c_str()) == 0) {
				set_COMmethod(mxa);
			}
			else if (strcmpi("DistanceExponent", field.c_str()) == 0) {
				set_DistanceExponent(mxa);
			}
			else if (strcmpi("GradientExponent", field.c_str()) == 0) {
				set_GradientExponent(mxa);
			}
			else if (strcmpi("RadiusCutoff", field.c_str()) == 0) {
				set_RadiusCutoff(mxa);
			}
			else if (strcmpi("CutoffFactor", field.c_str()) == 0) {
				set_CutoffFactor(mxa);
			}
			else if (strcmpi("LimFrac", field.c_str()) == 0) {
				set_LimFrac(mxa);
			}
			else if (strcmpi("roiList", field.c_str()) == 0) {
				// roiList requires special set
				set_roiList(mxa);
			}
			else {
				(*this)[field] = mxa;
			}
		}

	public:
		virtual ~RoiParameterMap() {};

		/** Default Constructor
		* Automatically adds fields:
		*	'xyMethod'
		*	'COMmethod'
		*	'DistanceFactor'
		*	'LimFrac'
		*	'roiList'
		*/
		RoiParameterMap() :extras::cmex::ParameterMxMap(false) {
			/// Add MAP Defaults
			extras::cmex::ParameterMxMap::operator[]("xyMethod").takeOwnership(cmex::MxObject("radialcenter"));
			extras::cmex::ParameterMxMap::operator[]("COMmethod").takeOwnership(cmex::MxObject("gradmag"));
			extras::cmex::ParameterMxMap::operator[]("DistanceExponent").takeOwnership(mxCreateDoubleScalar(default_DistanceExponent));
			extras::cmex::ParameterMxMap::operator[]("GradientExponent").takeOwnership(mxCreateDoubleScalar(default_GradientExponent));
			extras::cmex::ParameterMxMap::operator[]("RadiusCutoff").takeOwnership(mxCreateDoubleScalar(default_RadiusCutoff));
			extras::cmex::ParameterMxMap::operator[]("CutoffFactor").takeOwnership(mxCreateDoubleScalar(default_CutoffFactor));
			extras::cmex::ParameterMxMap::operator[]("LimFrac").takeOwnership(mxCreateDoubleScalar(0.2));

			//Create default, empty struct for roiList
			extras::cmex::ParameterMxMap::operator[]("roiList").takeOwnership(cmex::MxStruct(0, { "Window","UUID" }));
		}

		//! refefine setParameters to validate the arguments
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {

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

		const mxArray* get_roiList() const { return (*this)["roiList"]; }
		double get_LimFrac() const {return LimFrac;}
		XY_FUNCTION get_xyMethod() const { return xyMethod; }

	};

}}