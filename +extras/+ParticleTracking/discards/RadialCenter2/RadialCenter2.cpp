#include <extras/async/PersistentArgsProcessor.hpp>
#include "../radialcenter/source/radialcenter_mex.hpp" //radialcenter code
#include <tuple>


struct rcArgsType {
	extras::cmex::NumericArray<double> WIND = extras::cmex::MxObject::createPersistent(); //default to empty windows
	extras::cmex::NumericArray<double> GP = extras::cmex::MxObject::createPersistent(5); //default to GP=5
	rcdefs::RCparams rcParams;
	//extras::cmex::NumericArray<double> RadiusFilter; //default to empty RadiusFilter
	//rcdefs::COM_METHOD COMmethod = rcdefs::COM_METHOD::MEAN_ABS; //USE MEAN_ABS for center of mass
	//extras::cmex::NumericArray<double> XYc; //default to empty XY center guesses
	//double DistanceFactor = INFINITY; //Default to sharp radius filter cutoff
};

class RC2 :public extras::async::PersistentArgsProcessor<rcArgsType> {//extras::async::PersistentArgsProcessor{//
	typedef extras::async::PersistentArgsProcessor<rcArgsType>::TaskPairType TaskPairType;
protected:
	/// method for Processing Tasks in the task list
	virtual extras::cmex::mxArrayGroup ProcessTask(const TaskPairType& argPair)
	{

		auto out =
			extras::ParticleTracking::radialcenter<extras::cmex::NumericArray<double>>(
				argPair.first.getArray(0),
				argPair.second->WIND,
				argPair.second->GP,
				argPair.second->rcParams);

		//assemble output into array of mxArray*
		mxArray* plhs[4] = { nullptr,nullptr,nullptr,nullptr };
		plhs[0] = out[0];
		plhs[1] = out[1];
		plhs[2] = out[2];
		plhs[3] = out[3];

		return extras::cmex::mxArrayGroup(4, plhs);

	}
public:
	// redefine setParameters
	void setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {

		using namespace extras::cmex;

		// check that arguments were passed
		if (nrhs < 1) {
			throw(std::runtime_error("RC2::setPersistentArgs() requires arguments."));
		}

		/// Parse value pair inputs
		extras::cmex::MxInputParser Parser(false); //create non-case sensitive input parser
		Parser.AddParameter("WIND");
		Parser.AddParameter("GP");
		Parser.AddParameter("RadiusFilter"); //create empty parameter
		Parser.AddParameter("XYc");
		Parser.AddParameter("COMmethod", "meanABS");
		Parser.AddParameter("DistanceFactor", INFINITY);

		if (Parser.Parse(nrhs, prhs) != 0) {
			throw(std::runtime_error("RC2::setPersistentArgs(): Parameters specified incorrectly"));
		}

		// change parameters as needed
		rcArgsType oldArgs = *CurrentArgs;

		if (Parser.wasFound("WIND")) { oldArgs.WIND = MxObject::createPersistent(Parser("WIND"));  }
		if (Parser.wasFound("GP")) { oldArgs.GP = MxObject::createPersistent(Parser("GP")); }
		if (Parser.wasFound("RadiusFilter")) {
			oldArgs.rcParams.RadiusFilter = std::make_shared<extras::cmex::NumericArray<double>>(MxObject::createPersistent(Parser("RadiusFilter")));
		}
		if (Parser.wasFound("XYc")) {
			oldArgs.rcParams.XYc = std::make_shared<extras::cmex::NumericArray<double>>(MxObject::createPersistent(Parser("XYc")));
		}
		if (Parser.wasFound("COMmethod")){
			if (strcmpi("meanABS", extras::cmex::getstring(Parser("COMmethod")).c_str())) {
				oldArgs.rcParams.COMmethod = rcdefs::COM_METHOD::MEAN_ABS;
			}else if (strcmpi("gradMAG", extras::cmex::getstring(Parser("COMmethod")).c_str())) {
				oldArgs.rcParams.COMmethod = rcdefs::COM_METHOD::GRAD_MAG;
			}else if (strcmpi("normal", extras::cmex::getstring(Parser("COMmethod")).c_str())) {
				oldArgs.rcParams.COMmethod = rcdefs::COM_METHOD::NORMAL;
			}else {
				throw(std::runtime_error("RC2::setPersistentArgs(): invalid COMmethod"));
			}
		}
		if (Parser.wasFound("DistanceFactor")) { oldArgs.rcParams.DistanceFactor = fabs(mxGetScalar(Parser("DistanceFactor"))); }

		if (oldArgs.WIND.isempty() && oldArgs.rcParams.XYc->isempty()) {
			mexWarnMsgTxt("RC2::setPersistentArgs(): Both WIND and XYc are empty, the whole image will be used and a single symmetric center will be returned.");
		}

		CurrentArgs = std::make_shared<rcArgsType>(oldArgs);
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
			throw(runtime_error("RC2::pushTask() only accepts one input."));
		}

		if (CurrentArgs->WIND.isempty() && CurrentArgs->rcParams.XYc->isempty()) {
			mexWarnMsgTxt("RC2::pushTask() no WIND or XYc supplied, pushTask will process entire image and return a single xy coordinate");
		}


		// add task to the TaskList
		std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

		TaskList.emplace_back(
			std::make_pair(
				extras::cmex::mxArrayGroup(1,prhs),
				CurrentArgs
			)
		);

	}
};

//we must define the setPersistentArgs method for our custom PersistArgsType
template<> void extras::async::PersistentArgsProcessor<rcArgsType>::setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {};

extras::SessionManager::ObjectManager<RC2> manager;
extras::async::PersistentArgsProcessorInterface<RC2, manager> mex_interface; //create interface manager for the ExampleProcessor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	mex_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}
