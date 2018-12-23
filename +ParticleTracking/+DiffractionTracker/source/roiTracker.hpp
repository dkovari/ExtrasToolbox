#pragma once

#include <extras/Array.hpp>
#include <memory>
#include <extras/cmex/mxobject.hpp>
#include <extras/cmex/NumericArray.hpp>
#include "../../radialcenter/source/radialcenter_mex.hpp" //radialcenter code
#include "../../barycenter/source/barycenter.hpp" //barycenter code

#include <extras/async/PersistentArgsProcessor.hpp>

enum XY_FUNCTION{
    BARYCENTER,
    RADIALCENTER
};

struct roiListXY{
    // Other Globals
    XY_FUNCTION xyMethod = RADIALCENTER;

    std::shared_ptr<extras::cmex::MxObject> roiStruct; //original struct array object used to set roiList

    std::shared_ptr<extras::ArrayBase<double>> WIND = std::make_shared<extras::Array<double>>(); //array containing windows of the rois, formatted as [X1,X2,Y1,Y2]

    //////////////
    // For radialcenter
    std::shared_ptr<extras::ArrayBase<double>> XYc=std::make_shared<extras::Array<double>>(); //optional array containing XYc centroid quesses
	std::shared_ptr<extras::ArrayBase<double>> GP = std::make_shared<extras::Array<double>>(); //optional array containing GP for radialcenter
	std::shared_ptr<extras::ArrayBase<double>> RadiusFilter = std::make_shared<extras::Array<double>>(); //optional array containing RadiusFilter for radialcenter

    /// globals for radial center
	rcdefs::COM_METHOD COMmethod = rcdefs::MEAN_ABS;
	double DistanceFactor = INFINITY;

    /// globals for barycenter
    double barycenterLimFrac = 0.2;

//////////// METHODS

/// Set roiList Parameters using struct array
/// You can redefine this method to extend roiListXY to work with more data
/// in which case you should call this class's imstance first, and then your
/// additional code.
virtual void setFromStruct(const mxArray* srcStruct) {
    auto newStruct = std::make_shared<extras::cmex::MxObject>(srcStruct);
    if (!newStruct->isstruct()) {
        throw(std::runtime_error("roiAgregator::setFromStruct(): MxObject is not a struct"));
    }
    if (!extras::cmex::hasField(*newStruct, "Window")) {
        throw(std::runtime_error("roiAgregator::setFromStruct(): struct does not contain 'Window' field"));
    }
    if (!extras::cmex::hasField(*newStruct, "UUID")) {
        throw(std::runtime_error("roiAgregator::setFromStruct(): struct does not contain 'UUID' field"));
    }

    size_t len = newStruct->numel();
    //create temporary array that will be pushed into main variables at the end of the function
    // if an error is thrown, original arrays will remain unchanged.
    std::shared_ptr<extras::ArrayBase<double>> newWIND = std::make_shared<extras::Array<double>>(len, 4);
    std::shared_ptr<extras::ArrayBase<double>> newXYc = XYc;
    std::shared_ptr<extras::ArrayBase<double>> newGP = GP;
    std::shared_ptr<extras::ArrayBase<double>> newRadiusFilter = RadiusFilter;

    if (extras::cmex::hasField(*newStruct, "XYc")) {
        newXYc->resize(len, 2);
    }
    else {
        newXYc->resize(0, 2);
    }

    if (extras::cmex::hasField(*newStruct, "GP")) {
        newGP->resize(len);
    }
    else {
        newGP->resize(0,1);
    }
    if (extras::cmex::hasField(*newStruct, "RadiusFilter")) {
        newRadiusFilter->resize(len);
    }
    else {
        newRadiusFilter->resize(0, 1);
    }

    for (size_t n = 0; n < len; ++n) {
        // set window
        extras::cmex::NumericArray<double> Window(mxGetField(*newStruct,n,"Window"));
        (*newWIND)(n, 0) = Window[0]; //X1
        (*newWIND)(n, 1) = Window[0]+Window[2]-1; //X2=x+w-1
        (*newWIND)(n, 2) = Window[1]; //Y1
        (*newWIND)(n, 3) = Window[1] + Window[3] - 1; //Y2=y+h-1

        if (extras::cmex::hasField(*newStruct, "XYc")) {
            extras::cmex::NumericArray<double> sXYc(mxGetField(*newStruct, n, "XYc"));
            (*newXYc)(n, 0) = sXYc[0];
            (*newXYc)(n, 1) = sXYc[1];
        }

        if (extras::cmex::hasField(*newStruct, "GP")) {
            extras::cmex::NumericArray<double> sGP(mxGetField(*newStruct, n, "GP"));
            (*newGP)(n) = sGP[0];
        }
        if (extras::cmex::hasField(*newStruct, "RadiusFilter")) {
            extras::cmex::NumericArray<double> sRadiusFilter(mxGetField(*newStruct, n, "RadiusFilter"));
            (*newRadiusFilter)(n) = sRadiusFilter[0];
        }
    }

    // no errors!
    // set main variables
    newStruct->makePersistent();
    roiStruct = newStruct;
    WIND = newWIND;
    GP = newGP;
    XYc = newXYc;
    RadiusFilter = newRadiusFilter;

}

mxArray* getCopyOfRoiStruct() const{
    return mxDuplicateArray(*roiStruct);
}

size_t numberOfROI() const {
    return mxGetNumberOfElements(*roiStruct);
}


};

template <typename SettingsArgs=roiListXY>
class roiTrackerXY :public extras::async::PersistentArgsProcessor<SettingsArgs> {//extras::async::PersistentArgsProcessor{//
	typedef typename extras::async::PersistentArgsProcessor<SettingsArgs>::TaskPairType TaskPairType;
    typedef typename extras::async::PersistentArgsProcessor<SettingsArgs> ParentClass; //alias to templated parent class name
protected:
	/// method for Processing Tasks in the task list
    /// If you extend roiTrackerXY you should re-define this and use the original
    /// as a nested function
    /// Example:
    ///     ProcessTask(const TaskPairType& argPair){
    ///         mxArrayGroup out = roiTracker::ProcessTask(argPair);
    ///        mxArray* outStruct = out[0];
    ///         ...
    ///             DO YOUR OTHER TASKS, ADD RESULTS TO outStruct
    ///         ...
    ///         return out;
    ///     }
	virtual extras::cmex::mxArrayGroup ProcessTask(const TaskPairType& argPair)
	{
		//if no windows or XYc, then return empty
		if (argPair.second->WIND->isempty() && argPair.second->XYc->isempty()) {
			return extras::cmex::mxArrayGroup();
		}

		mxArray* outStruct = argPair.second->getCopyOfRoiStruct();

        int fn_xyM = mxAddField(outStruct, "xyMethod");
        if (fn_xyM < 0) { mxFree(outStruct); throw(std::runtime_error("could not add field='xyMethod' to output struct")); }


        switch(argPair.second->xyMethod){
            case RADIALCENTER:
            {
                //Do RadialCenter
        		rcdefs::RCparams rcP;
        		rcP.XYc = argPair.second->XYc;
        		rcP.RadiusFilter = argPair.second->RadiusFilter;
        		rcP.COMmethod = argPair.second->COMmethod;
        		rcP.DistanceFactor = argPair.second->DistanceFactor;

                std::vector<extras::Array<double>> rcOut =
                    extras::ParticleTracking::radialcenter<extras::Array<double>>(
                        argPair.first.getArray(0),
                        *(argPair.second->WIND),
                        *(argPair.second->GP),
                        rcP,4 );

        		// add X,Y,varXY,RWR_N to output struct
        		int fn_X = mxAddField(outStruct, "X");
        		if (fn_X < 0) { mxFree(outStruct); throw(std::runtime_error("could not add field='X' to output struct")); }
        		int fn_Y = mxAddField(outStruct, "Y");
        		if (fn_Y < 0) { mxFree(outStruct); throw(std::runtime_error("could not add field='Y' to output struct")); }
        		int fn_varXY = mxAddField(outStruct, "varXY");
        		if (fn_varXY < 0) { mxFree(outStruct); throw(std::runtime_error("could not add field='varXY' to output struct")); }
        		int fn_RWR_N = mxAddField(outStruct, "RWR_N");
        		if (fn_RWR_N < 0) { mxFree(outStruct); throw(std::runtime_error("could not add field='RWR_N' to output struct")); }

        		//set field values
        		for (size_t n = 0; n < argPair.second->numberOfROI(); ++n) {
        			mxSetFieldByNumber(outStruct, n, fn_X, mxCreateDoubleScalar(rcOut[0][n]));
        			mxSetFieldByNumber(outStruct, n, fn_Y, mxCreateDoubleScalar(rcOut[1][n]));
        			extras::cmex::NumericArray<double> vxy(2, 1);
        			vxy(0) = rcOut[2](n, 0);
        			vxy(1) = rcOut[2](n, 1);
        			mxSetFieldByNumber(outStruct, n, fn_varXY, vxy);
        			mxSetFieldByNumber(outStruct, n, fn_RWR_N, mxCreateDoubleScalar(rcOut[3][n]));

                    mxSetFieldByNumber(outStruct,n,fn_xyM,mxCreateString("radialcenter"));
        		}
            }
            break;
            case BARYCENTER:
            {

            }
            break;
        }



		return extras::cmex::mxArrayGroup(1, &outStruct);

	}
public:

    // sets settingsArgs for XY settings
    // If you are redefining this, remember that the roiStruct is set via
    // roiListXY::setFromStruct()
    // So if your roiList is different you should derive a new roiListXY class
    // and redefine setFromStruct()
    //
    // If you want to extent setPersistentArgs you should nest this method
    // and pass it the newSettings shared_ptr to your working-copy of the
    // settingsArgument...
    //
    // setPersistentArgs(nrhs,prhs){
    //      auto newSettings = std::make_shared<SettingsArgs>(*ParentClass::CurrentArgs); //make a copy of the roiSettings
    //      ...
    //      roiTrackerXY::setPersistentArgs(nrhs,prhs,newSettings);
    //      ...
    //      ParentClass::CurrentArgs = newSettings;
    // }
	virtual void setPersistentArgs(size_t nrhs, const mxArray* prhs[],std::shared_ptr<SettingsArgs>newSettings=nullptr) {
		/* Syntax
		setPersistentArgs(roiStruct)
		setPersistentArgs(roiStruct,'Param',val)
		setPersistentArgs('roiList',roiStruct','Param',val,...)
		setPersistentArgs('Param',val);
		*/

		// check that arguments were passed
		if (nrhs < 1) {
			throw(std::runtime_error("roiTrackerXY::setPersistentArgs() requires arguments."));
		}

		if (!mxIsStruct(prhs[0]) && !mxIsChar(prhs[0])) {
			throw(std::runtime_error("roiTrackerXY::setPersistentArgs() first arg must be struct or char array specifying parameter name"));
		}

        if(!newSettings){
            newSettings = std::make_shared<SettingsArgs>(*ParentClass::CurrentArgs); //make a copy of the roiSettings
        }

		bool found_struct = false;
		if (mxIsStruct(prhs[0])) {
			found_struct = true;
			newSettings->setFromStruct(prhs[0]);
		}

		/// Parse value pair inputs
		extras::cmex::MxInputParser Parser(false); //create non-case sensitive input parser
        Parser.IgnoreUnspecifiedParameters = true; //don't have error on Unspecified Parameters
		if (found_struct) {
			Parser.AddParameter("roiList");
		}
		Parser.AddParameter("COMmethod", "meanABS");
		Parser.AddParameter("DistanceFactor", INFINITY);
        Parser.AddParameter("LimFrac",0.2);


		if (Parser.Parse(nrhs-size_t(found_struct), &(prhs[size_t(found_struct)])) != 0) {
			throw(std::runtime_error("roiTrackerXY::setPersistentArgs(): Parameters specified incorrectly"));
		}
		if (!found_struct && Parser.wasFound("roiList")) {
			newSettings->setFromStruct(Parser("roiList"));
		}
		if (Parser.wasFound("COMmethod")) {
			if (strcmpi("meanABS", extras::cmex::getstring(Parser("COMmethod")).c_str())) {
				newSettings->COMmethod = rcdefs::COM_METHOD::MEAN_ABS;
			}
			else if (strcmpi("gradMAG", extras::cmex::getstring(Parser("COMmethod")).c_str())) {
				newSettings->COMmethod = rcdefs::COM_METHOD::GRAD_MAG;
			}
			else if (strcmpi("normal", extras::cmex::getstring(Parser("COMmethod")).c_str())) {
				newSettings->COMmethod = rcdefs::COM_METHOD::NORMAL;
			}
			else {
				throw(std::runtime_error("roiTrackerXY::setPersistentArgs(): invalid COMmethod"));
			}
		}

		if (Parser.wasFound("DistanceFactor")) { newSettings->DistanceFactor = fabs(mxGetScalar(Parser("DistanceFactor"))); }

        if (Parser.wasFound("LimFrac")) { newSettings->barycenterLimFrac = fabs(mxGetScalar(Parser("LimFrac"))); }

		//set the arguments
		ParentClass::CurrentArgs = newSettings;
	}

	/// push arguments to the task list.
	/// Each call to ProcessTask by the task thread will pop the "pushed" arguments
	/// off the stack and use them as arguments for ProcessTask(___)
	///
	/// Will warn if WIND and XYC are empty
	virtual void pushTask(size_t nrhs, const mxArray* prhs[])
	{
		using namespace std;
		if (nrhs < 1) { return; }
		if (nrhs > 1) {
			throw(runtime_error("XYtracker::pushTask() only accepts one input. It should be an image"));
		}

		// add task to the TaskList
		ParentClass::pushTask(nrhs,prhs);

	}
 };

//we must define the setPersistentArgs method for for parent processor type, even though we won't use it.
template<> void extras::async::PersistentArgsProcessor<roiListXY>::setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {};
