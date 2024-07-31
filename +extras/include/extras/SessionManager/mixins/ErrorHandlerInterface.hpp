#include "..\mexInterfaceManager.hpp"

namespace extras {namespace SessionManager {namespace mixins {

	template<class ObjType>
	class mexErrorHandlerInterface : virtual public mexInterfaceManager<ObjType> {
		typedef mexInterfaceManager<ObjType> ParentType;
	protected:
		virtual void wasErrorThrown(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs, prhs)->wasErrorThrown());
		}
		virtual void getError(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			std::exception_ptr err = ParentType::getObjectPtr(nrhs, prhs)->getError();

			if (err == nullptr) { //no errors, return empty
				plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
				return;
			}

			//convert exception ptr to struct
			try {
				rethrow_exception(err);
			}
			catch (const std::exception& e) {
				const char* fields[] = { "identifier","message" };
				mxArray* out = mxCreateStructMatrix(1, 1, 2, fields);
				mxSetField(out, 0, "identifier", mxCreateString("ProcessingError"));
				mxSetField(out, 0, "message", mxCreateString(e.what()));

				plhs[0] = out;
			}
		}
		virtual void clearError(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearError();
		}
	public:
		mexErrorHandlerInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("wasErrorThrown", std::bind(&mexErrorHandlerInterface::wasErrorThrown, this, _1, _2, _3, _4));
			ParentType::addFunction("getError", std::bind(&mexErrorHandlerInterface::getError, this, _1, _2, _3, _4));
			ParentType::addFunction("clearError", std::bind(&mexErrorHandlerInterface::clearError, this, _1, _2, _3, _4));
		}

		virtual ~mexErrorHandlerInterface() {
#ifdef _DEBUG
			mexPrintf("Destroying mexErrorHandlerInterface<%s,...>\n", typeid(ObjType).name());
			mexEvalString("pause(0.2)");
#endif
		}
		
	};

}}}