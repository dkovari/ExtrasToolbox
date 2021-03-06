/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/

#include <extras/async/ParamProcessor.hpp>

class ExampleProcessor : public extras::async::ParamProcessor {
protected:
	/// method for Processing Tasks in the task list
	extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& TaskArgs, std::shared_ptr<const extras::cmex::ParameterMxMap> Params) {

		if (!Params) { //Params were not initialized, just use an empty map
			Params = std::make_shared<extras::cmex::ParameterMxMap>();
		}
		// Create output containing args and parameters
	
		extras::cmex::mxArrayGroup out(TaskArgs.size()+2*Params->size());
		
		size_t k;
		for (k = 0; k < TaskArgs.size(); k++) {
			out.setArray(k, TaskArgs.getConstArray(k));
		}
		for (const auto& p : *Params) {
			out.setArray(k, extras::cmex::MxObject(p.first).getmxarray());
			out.setArray(k + 1, p.second.getConstArray());
			k += 2;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); //let some time pass
		return out;
	}
public:
	ExampleProcessor() :extras::async::ParamProcessor() {};
};

extras::SessionManager::ObjectManager<ExampleProcessor> manager;
extras::async::ParamProcessorInterface<ExampleProcessor, manager> ep_interface; //create interface manager for the ExampleProcessor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	ep_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}