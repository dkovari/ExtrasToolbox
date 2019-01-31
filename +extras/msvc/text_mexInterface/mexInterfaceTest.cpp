#include <extras/SessionManager/mexInterface.hpp>

struct myObj {
	myObj() {
		mexPrintf("Create myObj\n");
		mexEvalString("pause(0.5)");
	}

	void fn() {
		mexPrintf("myObj:fn() Press a key to continue\n");
		mexEvalString("pause()");
	}
};

extras::SessionManager::ObjectManager<myObj> manager;

class myObjInterface : public extras::SessionManager::mexInterface<myObj, manager> {

	void fn(int nlhs, mxArray*plhs[], int nrhs, const mxArray* prhs[]) {
		getObjectPtr(nrhs, prhs)->fn();
	}
public:
	myObjInterface() {
		using namespace std::placeholders;
		addFunction("fn",
			std::bind(&myObjInterface::fn, this, _1, _2, _3, _4)
		);
	}
};

myObjInterface mexI;

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
	//mexPrintf("Press key to continue\n");
	//mexEvalString("pause()");
	mexI.mexFunction(nlhs, plhs, nrhs, prhs);
}
