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
#include <radialcenter/source/radialcenter_mex.hpp>

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
	class RoiParameterMap : public extras::cmex::ParameterMxMap {
	protected:
		std::shared_ptr<extras::ArrayBase<double>> WIND = std::make_shared<extras::Array<double>>(0);
		std::shared_ptr<extras::ArrayBase<double>> XYc = std::make_shared<extras::Array<double>>(0);
		std::shared_ptr<extras::ArrayBase<double>> GP = std::make_shared<extras::Array<double>>(0);
		std::shared_ptr<extras::ArrayBase<double>> RadiusFilter = std::make_shared<extras::Array<double>>(0);

		rcdefs::COM_METHOD COMmethod = rcdefs::COM_METHOD::MEAN_ABS;
		double DistanceFactor = INFINITY;
		double LimFrac = 0.2;
		XY_FUNCTION xyMethod = XY_FUNCTION::RADIALCENTER;

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

		void set_DistanceFactor(const cmex::MxObject& value) {
			if (!value.isnumeric()) {
				throw("DistanceFactor must be numeric");
			}
			if (value.numel() != 1) {
				throw("DistanceFacor must be scalar numeric");
			}
			DistanceFactor = mxGetScalar(value);
			(*this)["DistanceFactor"] = value;
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
					cmex::NumericArray<double> Arr(pMa);

					// set window
					(*newXYc)(n, 0) = Arr[0];
					(*newXYc)(n, 1) = Arr[1];
				}
			}

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
			
			/////////////////////////////////////////////////
			// Made it here without errors, set the values
			
			WIND = newWIND;
			GP = newGP;
			XYc = newXYc;
			RadiusFilter = newRadiusFilter;
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
			else if (strcmpi("DistanceFactor", field.c_str()) == 0) {
				set_DistanceFactor(mxa);
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
			extras::cmex::ParameterMxMap::operator[]("COMmethod").takeOwnership(cmex::MxObject("meanABS"));
			extras::cmex::ParameterMxMap::operator[]("DistanceFactor").takeOwnership(mxCreateDoubleScalar(INFINITY));
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

		std::shared_ptr<extras::ArrayBase<double>> get_WIND() const { return WIND; }
		std::shared_ptr<extras::ArrayBase<double>> get_XYc() const { return XYc; }
		std::shared_ptr<extras::ArrayBase<double>> get_GP() const { return GP; }
		std::shared_ptr<extras::ArrayBase<double>> get_RadiusFilter() const { return RadiusFilter; }

		const mxArray* get_roiList() const { return (*this)["roiList"]; }

		rcdefs::COM_METHOD get_COMmethod() const { return COMmethod; }
		double get_DistanceFactor() const {return DistanceFactor;}
		double get_LimFrac() const { return LimFrac; }
		XY_FUNCTION get_xyMethod() const { return xyMethod; }
	};

}}