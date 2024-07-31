#pragma once
#include <extras/SessionManager/mexInterfaceManager.hpp>

namespace extras {namespace async {namespace mixins {

	template<class ObjType>
	class mexTaskQueueInterface : virtual public extras::SessionManager::mexInterfaceManager<ObjType> {
		typedef extras::SessionManager::mexInterfaceManager<ObjType> ParentType;
	protected:
		virtual void remainingTasks(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;
		virtual void running(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;
		virtual void cancelRemainingTasks(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;
		virtual void pause(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;
		virtual void resume(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;
		virtual void void cancelNextTask(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;
	public:
		mexTaskQueueInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("remainingTasks", std::bind(&mexTaskQueueInterface::remainingTasks, this, _1, _2, _3, _4));
			ParentType::addFunction("running", std::bind(&mexTaskQueueInterface::running, this, _1, _2, _3, _4));
			ParentType::addFunction("cancelRemainingTasks", std::bind(&mexTaskQueueInterface::cancelRemainingTasks, this, _1, _2, _3, _4));
			ParentType::addFunction("pause", std::bind(&mexTaskQueueInterface::pause, this, _1, _2, _3, _4));
			ParentType::addFunction("resume", std::bind(&mexTaskQueueInterface::resume, this, _1, _2, _3, _4));
			ParentType::addFunction("cancelNextTask", std::bind(&mexTaskQueueInterface::cancelNextTask, this, _1, _2, _3, _4));
		}

	};

}}}