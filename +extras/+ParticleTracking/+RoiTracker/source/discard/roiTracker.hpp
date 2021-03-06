#pragma once

#include <extras/Array.hpp>
#include <memory>
#include <extras/cmex/mxobject.hpp>
#include <extras/cmex/NumericArray.hpp>
#include "../../radialcenter/source/radialcenter_mex.hpp" //radialcenter code
#include "../../barycenter/source/barycenter_mex.hpp" //barycenter code
#include <extras/string_extras.hpp>

#include <extras/async/PersistentArgsProcessor.hpp>

#include "roiObject.hpp"

/*// Struct used to store roi window and tracking parameters
struct roiListXY{
    // Other Globals
    XY_FUNCTION xyMethod = RADIALCENTER; ///< xy tracking algorithm

    std::shared_ptr<extras::cmex::MxObject> roiStruct = std::make_shared<extras::cmex::MxObject>(nullptr); ///< original struct array object used to set roiList

    std::shared_ptr<extras::ArrayBase<double>> WIND = std::make_shared<extras::Array<double>>(); ///< array containing windows of the rois, formatted as [x0,y0,w,h]

    //////////////
    // For radialcenter
    std::shared_ptr<extras::ArrayBase<double>> XYc=std::make_shared<extras::Array<double>>(); ///< optional array containing XYc centroid quesses
	std::shared_ptr<extras::ArrayBase<double>> GP = std::make_shared<extras::Array<double>>(); ///< optional array containing GP for radialcenter
	std::shared_ptr<extras::ArrayBase<double>> RadiusFilter = std::make_shared<extras::Array<double>>(); ///< optional array containing RadiusFilter for radialcenter

    /// globals for radial center
	rcdefs::COM_METHOD COMmethod = rcdefs::MEAN_ABS; ///< method for calculating image center of mass (used by radialcenter)
	double DistanceFactor = INFINITY; ///< radial filter logistic factor, Inf=> sharp cutoff (used by radialcenter)

    /// globals for barycenter
    double barycenterLimFrac = 0.2; ///< LimFrac used by barycenter

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

		// srtStruct is ok, duplicate and make persistent
		newStruct->managedata();
		newStruct->makePersistent();

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
			(*newWIND)(n, 0) = Window[0]-1; //x0 fixed for 0-indexing
			(*newWIND)(n, 1) = Window[1]-1; //y0 fixed for 0-indexing
			(*newWIND)(n, 2) = Window[2]; //w
			(*newWIND)(n, 3) = Window[3]; //h

			if (extras::cmex::hasField(*newStruct, "XYc")) {
				extras::cmex::NumericArray<double> sXYc(mxGetField(*newStruct, n, "XYc"));
				(*newXYc)(n, 0) = sXYc[0]-1; //fixed for 0-indexing
				(*newXYc)(n, 1) = sXYc[1]-1; //fixed for 0-indexing
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
		//newStruct->makePersistent();
		roiStruct = newStruct;
		WIND = newWIND;
		GP = newGP;
		XYc = newXYc;
		RadiusFilter = newRadiusFilter;
	}

	/// Returns a copy of the mxArray struct used to set the roi parameters
	mxArray* getCopyOfRoiStruct() const{
		return mxDuplicateArray(*roiStruct);
	}

	/// number of roi windows
	size_t numberOfROI() const {
		return mxGetNumberOfElements(*roiStruct);
	}

	///
	~roiListXY() {
		roiStruct.reset();
		WIND.reset();
	}

};
*/

template <typename SettingsArgs=roiAgregator>
class roiTrackerXY :public extras::async::PersistentArgsProcessor<SettingsArgs> {//extras::async::PersistentArgsProcessor{//
	typedef typename extras::async::PersistentArgsProcessor<SettingsArgs>::TaskPairType TaskPairType; //alias or pair containing mxArrayGroup and std::shared_ptr<SettingsArgs>
    typedef typename extras::async::PersistentArgsProcessor<SettingsArgs> ParentClass; //alias to templated parent class name
    typedef typename std::shared_ptr<SettingsArgs> SettingsArgs_Ptr;
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

        const extras::cmex::mxArrayGroup& imgGroup = argPair.first;
        const SettingsArgs_Ptr& settings = argPair.second;

        using namespace extras::cmex;

		//if no windows or XYc, then return empty
		if (settings->isempty() && settings->XYc->isempty()) {
			return mxArrayGroup();
		}

        /// Get Image
        const mxArray* img=nullptr;
        const mxArray* time=nullptr;
        if(mxIsStruct(argPair.first.getArray(0))){
            img = mxGetField(argPair.first.getArray(0),0,"ImageData");
            time = mxGetField(argPair.first.getArray(0),0,"Time");
        }else{
            img=argPair.first.getArray(0);
        }

        MxStruct outStruct(argPair.second->getCopyOfRoiStruct(),false,true); //construct output struct;

        // process images
        switch(argPair.second->xyMethod){
            case RADIALCENTER:
            {
                //Do RadialCenter
        		rcdefs::RCparams rcP;
        		rcP.XYc = argPair.second->XYc;
        		rcP.RadiusFilter = argPair.second->RadiusFilter;
        		rcP.COMmethod = argPair.second->COMmethod;
        		rcP.DistanceFactor = argPair.second->DistanceFactor;

                auto rcOut =
                    extras::ParticleTracking::radialcenter<extras::Array<double>>(
                        img,
                        *(argPair.second->WIND),
                        *(argPair.second->GP),
                        rcP);

                rcOut[0]+=1; //shift for 1-indexing
                rcOut[1]+=1; //shift for 1-indexing

        		//set field values
        		for (size_t n = 0; n < argPair.second->numberOfROI(); ++n) {
                    outStruct(n,"X") = rcOut[0][n];
                    outStruct(n,"Y") = rcOut[1][n];

                    NumericArray<double> vxy(2, 1);
        			vxy(0) = rcOut[2](n, 0);
        			vxy(1) = rcOut[2](n, 1);
                    outStruct(n,"varXY") = vxy;

                    outStruct(n,"RWR_N") = rcOut[3][n];
                    outStruct(n,"xyMethod") = "radialcenter";
        		}
            }
            break;
            case BARYCENTER:
            { // Do Barycenter

				auto bcOut = extras::ParticleTracking::barycenter_mx<extras::Array<double>>(
                    img,
                    *(argPair.second->WIND),
                    argPair.second->barycenterLimFrac);

                bcOut[0]+=1;
                bcOut[1]+=1;

                //set field values
        		for (size_t n = 0; n < argPair.second->numberOfROI(); ++n) {
                    outStruct(n,"X") = bcOut[0][n];
                    outStruct(n,"Y") = bcOut[1][n];

                    outStruct(n,"xyMethod") = "radialcenter";
        		}

            }
            break;
        }

        //// Finalize outStruct
        if(time!=nullptr){
            int fn_Time = mxAddField(outStruct, "Time");
            if (fn_Time < 0) { mxFree(outStruct); throw(std::runtime_error("could not add field='Time' to output struct")); }
            for(size_t n=0;n<argPair.second->numberOfROI();++n){
                mxSetFieldByNumber(outStruct,n,fn_Time,mxDuplicateArray(time));
            }
        }

        mxArray* outMx = outStruct.releasemxarray();
		return extras::cmex::mxArrayGroup(1, &outMx); //wrap output struct in mxArrayGroup (makes Array persistent)

	}
public:

	roiTrackerXY() {
		//mexPrintf("Creating roiTrackerXY\n");

	}

	virtual ~roiTrackerXY() {
		//mexPrintf("Deleteing roiTrackerXY\n");

	}

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
	virtual void setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {
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

        SettingsArgs_Ptr newSettings = std::make_shared<SettingsArgs>(*ParentClass::CurrentArgs); // make a copy of CurrentArgs


		bool found_struct = false;
		if (mxIsStruct(prhs[0])) {
			found_struct = true;
			newSettings->setFromStruct(prhs[0]);
		}

		/// Parse value pair inputs
		extras::cmex::MxInputParser Parser(false); //create non-case sensitive input parser
        Parser.IgnoreUnspecifiedParameters = true; //don't have error on Unspecified Parameters
		if (!found_struct) { //didn't find roiList struct in first argument, look for name-value pair
			Parser.AddParameter("roiList");
		}
		Parser.AddParameter("COMmethod", "meanABS");
		Parser.AddParameter("DistanceFactor", INFINITY);
        Parser.AddParameter("LimFrac",0.2);
		Parser.AddParameter("xyMethod", "radialcenter");


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

		if (Parser.wasFound("xyMethod")) {
			newSettings->xyMethod = xyFunction(extras::cmex::getstring(Parser("xyMethod")).c_str());
		}

		//set the arguments
		ParentClass::CurrentArgs = newSettings;
	}

	/// push arguments to the task list.
	/// Each call to ProcessTask by the task thread will pop the "pushed" arguments
	/// off the stack and use them as arguments for ProcessTask(___)
	virtual void pushTask(size_t nrhs, const mxArray* prhs[])
	{
		using namespace std;
		if (nrhs < 1) { return; }
		if (nrhs > 1) {
			throw(runtime_error("XYtracker::pushTask() only accepts one input. It should be an image, or a struct containing 'ImageData' and 'Time' fields"));
		}

        if(mxIsStruct(prhs[0])){
            if(mxGetFieldNumber(prhs[0], "ImageData")<0){
                throw(std::runtime_error("XYtracker::pushTask(), Image structuct must contain 'ImageData' field"));
            }
            if(mxGetFieldNumber(prhs[0], "Time")<0){
                throw(std::runtime_error("XYtracker::pushTask(), Image structuct must contain 'Time' field"));
            }
        }

		// add task to the TaskList
		ParentClass::pushTask(nrhs,prhs);

	}

	/// implement re-definable getPersistentArgs()
	/// if you derive from roiTrackerXY using SettingsArgs != roiListXY
	/// you will need to define
	///  extras::cmex::MxCellArrayroiTrackerXY<YOUR_SETTINGS_TYPE>::getPersistentArgs() const {}
	///  somewhere in your cpp code, even though you won't ever use that implementation
	/// you will also need to define your own implementation of getPersistentArgs() in your derived class
	/// Example:
	///		class YourClass: public roiTrackerXY<YOUR_TYPE>{
	///			...
	///			extras::cmex::MxCellArray getPersistentArgs() const{
	///				... //your code here
	///			}
	///		};
	///		extras::cmex::MxCellArrayroiTrackerXY<YOUR_TYPE>::getPersistentArgs() const {return nullptr;} //define dummy (unused getPersistentArgs() implementation)
	virtual extras::cmex::MxCellArray getPersistentArgs() const;
 };
 /// Implement getPersistentArgs for mxArrayGroup type args
 /// Returns the argumens as Name-Value pairs, organized as a {2 x n} cell array
 template<> extras::cmex::MxCellArray roiTrackerXY<roiAgregator>::getPersistentArgs() const {
	 extras::cmex::MxCellArray out;
	 out.reshape(2, 5);

	 out(0) = "roiList";
	 out(1) = ParentClass::CurrentArgs->roiStruct->getmxarray();

	 out(2) = "COMmethod";
	 switch (ParentClass::CurrentArgs->COMmethod) {
	 case rcdefs::COM_METHOD::MEAN_ABS:
		 out(3) = "meanABS";
		 break;
	 case rcdefs::COM_METHOD::GRAD_MAG:
		 out(3) = "gradMAG";
		 break;
	 case rcdefs::COM_METHOD::NORMAL:
		 out(3) = "normal";
		 break;
	 }

	 out(4) = "DistanceFactor";
	 out(5) = ParentClass::CurrentArgs->DistanceFactor;

	 out(6) = "LimFrac";
	 out(7) = ParentClass::CurrentArgs->barycenterLimFrac;

	 out(8) = "xyMethod";
	 out(9) = xyFunctionName(ParentClass::CurrentArgs->xyMethod);

	 return out;
 }

//we must define the setPersistentArgs & getPersistentArgs methods for for parent processor type, even though we won't use it.
template<> void extras::async::PersistentArgsProcessor<roiAgregator>::setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {};

template<> extras::cmex::MxCellArray extras::async::PersistentArgsProcessor<roiAgregator>::getPersistentArgs() const { return extras::cmex::MxCellArray(); }
