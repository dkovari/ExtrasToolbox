/*
    Example AsyncProcessor Object

    This mex file creates a dummy processor which simply copies the task inputs
    provided via pushTask(Arg1,Arg2,...) to the results queue
    yielding a result
        [Arg1,Arg2,...] = popResult();
*/

#include <extras/async/AsyncProcessor.hpp>

class ExampleProcessor: public extras::async::AsyncProcessor{
protected:
    /// method for Processing Tasks in the task list
    virtual extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& args){
        extras::cmex::mxArrayGroup out = args;
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //let some time pass
        return out;
    }
};

extras::SessionManager::ObjectManager<ExampleProcessor> manager;
extras::async::AsyncMexInterface<ExampleProcessor,manager> ep_interface; //create interface manager for the ExampleProcessor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
    ep_interface.mexFunction(nlhs,plhs,nrhs,prhs);
}
