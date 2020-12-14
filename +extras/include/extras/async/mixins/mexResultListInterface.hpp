#pragma once
#include <extras/SessionManager/mexInterfaceManager.hpp>

namespace extras {namespace async {namespace mixins{

	template<class ObjType>
	class mexResultsListInterface : virtual public extras::SessionManager::mexInterfaceManager<ObjType> {
		typedef extras::SessionManager::mexInterfaceManager<ObjType> ParentType;
	protected:
		virtual void availableResults(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateDoubleScalar((double)(ParentType::getObjectPtr(nrhs, prhs)->availableResults()));
		}

		/// @brief PURE VIRTUAL method for returning number of output args of next result in result list
		/// YOU MUST DEFINE THIS METHOD!
		/// @param nlhs 
		/// @param plhs 
		/// @param nrhs 
		/// @param prhs 
		virtual void numResultOutputArgs(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;

		/// @brief PURE VIRTUAL method for returning next result in the results list
		/// YOU MUST DEFINE THIS METHOD!
		/// @param nlhs 
		/// @param plhs 
		/// @param nrhs 
		/// @param prhs 
		virtual void popResult(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) = 0;

		virtual void clearResults(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearResults();
		}

	public:
		mexResultsListInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("availableResults", std::bind(&mexResultsListInterface::availableResults, this, _1, _2, _3, _4));
			ParentType::addFunction("numResultOutputArgs", std::bind(&mexResultsListInterface::numResultOutputArgs, this, _1, _2, _3, _4));
			ParentType::addFunction("popResult", std::bind(&mexResultsListInterface::popResult, this, _1, _2, _3, _4));
			ParentType::addFunction("clearResults", std::bind(&mexResultsListInterface::clearResults, this, _1, _2, _3, _4));
		}

		virtual ~mexResultsListInterface() {
#ifdef _DEBUG
			mexPrintf("Destroying mexResultsListInterface<%s,...>\n", typeid(ObjType).name());
			mexEvalString("pause(0.2)");
#endif
		}
	};

}}}