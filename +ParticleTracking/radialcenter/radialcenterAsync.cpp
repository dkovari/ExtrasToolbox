/*
    radialcenterAsync Object

    This mex file creates a processor which applies the radialcenter routine asynchrounously.

    Usage:

    >> rcA = radialcenterAsync(); %create async processor
    >> rcA.pushTask(IMAGE,...Other Args...); %process IMAGE using specified arguments
    >> ... %do other things and wait for processor
    >> [x,y,varXY,d2] = popResults();
    >> rcA.setPersistentArgs(WIND,GP); %sets persistent arguments
    >> rcA.pushTask(IMAGE2); %push IMAGE and persistent WIND,GP
    >> rcA.pushTask(IMAGE3); %push another image, also using persistent Args
    >> rcA.pushTask(IMAGE4,WIND4,GP4); %causes error since persisten arguements are set
*/

#include <extras/cmex/NumericArray.hpp>

#include <extras/async/PersistentArgsProcessor.hpp>
//#include <extras/async/AsyncProcessor.hpp>
#include "source/radialcenter_mex.hpp"

#include <extras/SessionManager/ObjectManager.h> // Object manager includes
#include <extras/SessionManager/mexDispatch.h>

#include <vector>

class rcProc:public extras::async::PersistentArgsProcessor<> {//extras::async::PersistentArgsProcessor{//
	typedef extras::async::PersistentArgsProcessor<>::TaskPairType TaskPairType;
protected:
    /// method for Processing Tasks in the task list
    virtual extras::cmex::mxArrayGroup ProcessTask(const TaskPairType& argPair)
	{
        using namespace std;
        


		size_t sz = argPair.first.size() + argPair.second->size();

		std::vector<const mxArray*> vA;
		vA.reserve(sz);

#ifdef _DEBUG
		mexPrintf("rcProc::ProcessTask()\n");
		mexPrintf("\t argPair.first.size(): %d\n", argPair.first.size());
		mexPrintf("\t argPair.second->size(): %d\n", argPair.second->size());
#endif
        
        for(size_t n=0;n<argPair.first.size();++n){
#ifdef _DEBUG
			mexPrintf("\t arg1.getArray(%d): %p\n", n, argPair.first.getArray(n));
#endif
			vA.push_back(argPair.first.getArray(n));
#ifdef _DEBUG
			mexPrintf("\t vA.back(): %p\n", vA.back());
#endif
        }
        for(size_t n=0;n<argPair.second->size();++n){
#ifdef _DEBUG
			mexPrintf("\t argPair.second->getArray(%d): %p\n", n, argPair.second->getArray(n));
#endif
			vA.push_back(argPair.second->getArray(n));
#ifdef _DEBUG
			mexPrintf("\t vA.back(): %p\n", vA.back());
#endif
        }

#ifdef _DEBUG
		mexPrintf("\tentering radialcenter\n");
#endif

		mxArray* plhs[4] = { nullptr,nullptr,nullptr,nullptr};
		extras::ParticleTracking::radialcenter_mex(4, plhs, sz, vA.data());
#ifdef _DEBUG
		mexPrintf("\tfinished radialcenter\n");
#endif

        return extras::cmex::mxArrayGroup(4,plhs);
    }

#ifdef _DEBUG
public:
	virtual ~rcProc() {
		mexPrintf("~rcProc()\n");
	}
#endif
};


extras::SessionManager::ObjectManager<rcProc> manager;
extras::async::PersistentArgsProcessorInterface<rcProc, manager> rcProc_interface; //create interface manager for the ExampleProcessor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	rcProc_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}