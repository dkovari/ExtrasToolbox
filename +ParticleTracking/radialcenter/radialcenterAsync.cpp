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

#include <extras/async/ProcessorPersistentArgs.hpp>
//#include <extras/async/AsyncProcessor.hpp>
#include "source/radialcenter_mex.hpp"

#include <extras/SessionManager/ObjectManager.h> // Object manager includes
#include <extras/SessionManager/mexDispatch.h>

class rcProc:public extras::async::AsyncProcessor {//extras::async::ProcessorWithPersistentArgs{//
protected:
    /// method for Processing Tasks in the task list
    virtual extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& args)//const std::pair<extras::cmex::mxArrayGroup,std::shared_ptr<extras::cmex::mxArrayGroup>>& args)
	{
        using namespace std;
        extras::cmex::mxArrayGroup out(4);

		/*
		// DEBUG
		throw(runtime_error(string("in ProcessTask  ")+
			string("nTaskArgs: ") + to_string(args.first.size())+string(" ParamArgs:")+to_string(args.second->size())
		));

        const mxArray* *pA;
        size_t sz = args.first.size()+args.second->size();
        pA = new const mxArray* [sz];

        for(size_t n=0;n<args.first.size();++n){
            try{
                pA[n] = args.first.getArray(n);
            }catch(const exception& e){
                throw(runtime_error(
                    string(e.what())+string(" sz:")+to_string(sz)+string(" Arg1: ")+to_string(n)
                ));
            }
        }
        for(size_t n=0;n<args.second->size();++n){
            try{
                pA[n+args.first.size()] = args.second->getArray(n);
            }catch(const exception& e){
                throw(runtime_error(
                    string(e.what())+string(" sz:")+to_string(sz)+string(" Arg2: ")+to_string(n)
                ));
            }
        }

        extras::ParticleTracking::radialcenter_mex(4, out, sz, pA);

        delete[] pA;
		*/

		extras::ParticleTracking::radialcenter_mex(4, out, args.size(), args);


        return out;
    }
};
