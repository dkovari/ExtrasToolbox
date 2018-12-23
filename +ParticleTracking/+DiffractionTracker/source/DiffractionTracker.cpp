/* Expected Syntax
% setPersistentArgs(Name,Value)
%
% Parameters
% ----------------------------------------------------------------------
%   'ROIstruct',struct
%       struct array specifying ROI information
%       number of elements corresponds with number of particles/windows to track
%       Fields:
%           .UUID = '...' char array specifying uniquie identifier for
%                         ROI
%           .Window = [x,y,w,h] roi window
%           .RadiusFilter = ## size of radius filter to use for radial
%                              center
%           .MinRadius = ## minimum radius (in pixels) to use when
%                           creating the radial average
%           .MaxRadius = ## maximum radius (in pixels) to use when
%                           creating the radial average
%           .BinWidth = ## (default = 1) bin width (in pixels) for
%                          radial average
%           .ReferenceUUID = '...' or {'...','...',...} uuid of
%                            particles to use as reference
%           .IsCalibrated = t/f flag if particle has a look-up-table
%           .Zspline = pp spline describing LUT, used by splineroot
%           .dZspline = dpp derivative of Zspline, used by splineroot
% ----------------------------------------------------------------------
% Global Parameters:
%   'COMmethod', ''
%   'DistanceFactor'
%   'splineroot_TOL'
%   'splineroot_minStep'
%   'splineroot_maxItr'
%   'splineroot_min_dR2frac'
*/

#include "roiObject.hpp"
#include <extras/async/PersistentArgsProcessor.hpp>
#include "../../radialcenter/source/radialcenter_mex.hpp" //radialcenter code

class DiffractionTracker :public extras::async::PersistentArgsProcessor<roiAgregator> {//extras::async::PersistentArgsProcessor{//
	typedef extras::async::PersistentArgsProcessor<roiAgregator>::TaskPairType TaskPairType;
protected:
	/// method for Processing Tasks in the task list
	virtual extras::cmex::mxArrayGroup ProcessTask(const TaskPairType& argPair)
	{
		//if no windows or XYc, then return empty
		if (argPair.second->WIND->isempty() && argPair.second->XYc->isempty()) {
			return extras::cmex::mxArrayGroup();
		}

		mxArray* outStruct = argPair.second->getCopyOfRoiStruct();

		//Do RadialCenter
		rcdefs::RCparams rcP;
		rcP.XYc = argPair.second->XYc;
		rcP.RadiusFilter = argPair.second->RadiusFilter;
		rcP.COMmethod = argPair.second->COMmethod;
		rcP.DistanceFactor = argPair.second->DistanceFactor;

		auto rcOut = extras::ParticleTracking::radcen(argPair.first.getArray(0),
			*(argPair.second->WIND),
			*(argPair.second->GP),
			rcP,
			4);

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
		}

		return extras::cmex::mxArrayGroup(1, &outStruct);

	}
public:
	// redefine setParameters
	void setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {
		/* Syntax
		setPersistentArgs(roiStruct)
		setPersistentArgs(roiStruct,'Param',val)
		setPersistentArgs('roiList',roiStruct','Param',val,...)
		setPersistentArgs('Param',val);
		*/

		// check that arguments were passed
		if (nrhs < 1) {
			throw(std::runtime_error("DiffractionTracker::setPersistentArgs() requires arguments."));
		}

		if (!mxIsStruct(prhs[0]) && !mxIsChar(prhs[0])) {
			throw(std::runtime_error("DiffractionTracker::setPersistentArgs() first arg must be struct or char array specifying parameter name"));
		}

		std::shared_ptr<roiAgregator> newSettings = std::make_shared<roiAgregator>(*CurrentArgs); //make a copy of the roiSettings

		bool found_struct = false;
		if (mxIsStruct(prhs[0])) {
			found_struct = true;
			newSettings->setFromStruct(prhs[0]);
		}

		/// Parse value pair inputs
		extras::cmex::MxInputParser Parser(false); //create non-case sensitive input parser
		if (found_struct) {
			Parser.AddParameter("roiList");
		}
		Parser.AddParameter("COMmethod", "meanABS");
		Parser.AddParameter("DistanceFactor", INFINITY);


		if (Parser.Parse(nrhs-size_t(found_struct), &(prhs[size_t(found_struct)])) != 0) {
			throw(std::runtime_error("DiffractionTracker::setPersistentArgs(): Parameters specified incorrectly"));
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
				throw(std::runtime_error("DiffractionTracker::setPersistentArgs(): invalid COMmethod"));
			}
		}

		if (Parser.wasFound("DistanceFactor")) { newSettings->DistanceFactor = fabs(mxGetScalar(Parser("DistanceFactor"))); }

		//set the arguments
		CurrentArgs = newSettings;
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
			throw(runtime_error("DiffractionTracker::pushTask() only accepts one input. It should be an image"));
		}

		// add task to the TaskList
		std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

		TaskList.emplace_back(
			std::make_pair(
				extras::cmex::mxArrayGroup(1, prhs),
				CurrentArgs
			)
		);

	}
 };

//we must define the setPersistentArgs method for for parent processor type, even though we won't use it.
template<> void extras::async::PersistentArgsProcessor<roiAgregator>::setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {};

extras::SessionManager::ObjectManager<DiffractionTracker> manager;
extras::async::PersistentArgsProcessorInterface<DiffractionTracker, manager> mex_interface; //create interface manager for the processor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	mex_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}
