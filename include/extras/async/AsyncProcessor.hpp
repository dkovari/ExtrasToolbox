#pragma once

#include <mex.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <list>

#include <extras/cmex/mxArrayGroup.hpp>
#include <extras/SessionManager/mexInterface.hpp>

namespace extras{namespace async{

    /// Abstract class defining an Asynchronous Processor object
    class AsyncProcessor{
    protected:
        std::thread mThread;
        std::atomic<bool> ProcessAndEnd; //flag specifying that all remaining processes should be run
        std::atomic<bool> StopProcessingNow; //flag specifying that thread should stop immediately
        std::atomic<bool> ProcessRunning; //flag if thread is running

        std::atomic<bool> ErrorFlag;
        std::mutex LastErrorMutex;
        std::exception_ptr LastError=nullptr;

        std::mutex TaskListMutex;///mutex lock for accessing TaskList
        std::list<cmex::mxArrayGroup> TaskList; //list of remaining tasks

        std::mutex ResultsListMutex; ///mutex lock for accessing ResultsList
        std::list<cmex::mxArrayGroup> ResultsList; ///results list

		///Overloadable virtual method for Processing Tasks in the task list
        virtual cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) = 0;

        /// Method called in processing thread to execute tasks
        virtual void ProcessLoop(){
			using namespace std;
            try
            {
				while(!ProcessAndEnd && !StopProcessingNow){ //enter thread loop, unless thread is stopped and restared, we won't re-enter this loop
                    bool keep_proc = true;
                    while (keep_proc && !StopProcessingNow){
                        std::lock_guard<std::mutex> lock(TaskListMutex);
                        if(TaskList.size() > 0){
                            auto& task = TaskList.front();

							throw(runtime_error(string("in Async...:pushTask ") +
								string("nTaskArgs: ") + to_string(task.size())
							));

                            //DO Task
                            auto res = ProcessTask(task);
                            std::lock_guard<std::mutex> rlock(ResultsListMutex);

                            if(res.size()>0){
                                ResultsList.push_front( std::move(res) );
                            }

                            TaskList.pop_front();
                        }

                        if(TaskList.size() < 1){
                            keep_proc = false;
                        }
                    }
                }

            }catch (...){
                ProcessRunning = false;
                ErrorFlag = true;
                std::lock_guard<std::mutex> lock(LastErrorMutex);
                LastError = std::current_exception();
                return;
            }
        }

        virtual void StartProcessor() {
            ErrorFlag = false;
            StopProcessingNow = false;
            ProcessAndEnd = false;
            mThread = std::thread(&AsyncProcessor::ProcessLoop,this);

            ProcessRunning = true;
        };
        virtual void ProcessTasksAndEnd() {
            ProcessAndEnd = true;
            std::this_thread::sleep_for(std::chrono::microseconds(1000)); //let some time pass
            if (mThread.joinable()) {
				mexPrintf("\tWaiting for mex::AsyncProcessor to finish\n");
				mexEvalString("pause(0.2)");
				mThread.join();
			}
            ProcessRunning = false;
        }
        virtual void StopProcessor(){
            StopProcessingNow = true;
            std::this_thread::sleep_for(std::chrono::microseconds(1000)); //let some time pass
            if (mThread.joinable()) {
				mexPrintf("\tHalting mex::AsyncProcessor\n");
				mexEvalString("pause(0.2)");
				mThread.join();
			}
            ProcessRunning = false;
        }

    public:
        // Destructor
        ~AsyncProcessor(){
            ProcessTasksAndEnd();
        }

        //Constructor
        AsyncProcessor(){
            ProcessRunning = false;
            StartProcessor();
        }

        void pause() {
            StopProcessor();
        }

        void resume(){
            if(!ProcessRunning){
                StartProcessor();
            }
        }

        virtual void cancelRemainingTasks(){
            StopProcessor();

            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list
            TaskList.clear();

            mexPrintf("\t\t Remaining tasks cleared.\n");
            mexEvalString("pause(0.2)");
        }

        virtual size_t remainingTasks() const {return TaskList.size();}
        size_t availableResults() const {return ResultsList.size();}
        bool running() const {return ProcessRunning;}

        virtual void pushTask(size_t nrhs, const mxArray* prhs[])
        {
            //Convert mxArray list to array group
            cmex::mxArrayGroup AG(nrhs,prhs);

            // add task to the TaskList
            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

            TaskList.emplace_back(std::move(AG)); //move task to back of task list
        }

        virtual cmex::mxArrayGroup popResult(){
            if(ResultsList.size()<1){
                throw(std::runtime_error("No results in the ResultsList, cannot popResult()"));
            }

            //pop results
            std::lock_guard<std::mutex> lock(ResultsListMutex); //lock list
			cmex::mxArrayGroup out = std::move(ResultsList.back());
            ResultsList.pop_back();
            return out;
        }

        virtual void clearResults(){
            std::lock_guard<std::mutex> lock(ResultsListMutex); //lock list
            ResultsList.clear();
        }

        /// number of results output arguments for next result
        virtual size_t numResultOutputArgs() const{
            if(ResultsList.size()<1){
                return 0;
            }else{
                return ResultsList.back().size();
            }
        }

        /// true if there was an error
        virtual bool wasErrorThrown() const{
            return ErrorFlag;
        }

        /// Clears the error pointer and error flag.
        /// After calling ErroFlag=false and LastError=nullptr
        virtual void clearError() {
            ErrorFlag = false;
            std::lock_guard<std::mutex> lock(LastErrorMutex);
            LastError = nullptr;
        }

        ///check for errors thrown within the processing loop
		///returns exception_ptr to the thrown exception
		///if there aren't any exceptions, returns nullptr
		virtual std::exception_ptr getError() const {
			//read exception flags
			//see discussion here: https://stackoverflow.com/questions/41288428/is-stdexception-ptr-thread-safe
			if (ErrorFlag) { //yes, exception was thrown
				return LastError;
			}
			else {//no exception, return nullptr
				return nullptr;
			}
		}

    };

    /// Implement mexInterface for AsyncProcessor
    template<class ObjClass> /*ObjClass should be a derivative of AsyncProcessor*/
    class AsyncInterface: public extras::SessionManager::mexInterface<ObjClass>{
    protected:
        void remainingTasks(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateDoubleScalar(getObjectPtr(nrhs,prhs)->remainingTasks());
        }
        void availableResults(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateDoubleScalar(getObjectPtr(nrhs,prhs)->availableResults());
        }
        void running(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateLogicalScalar(getObjectPtr(nrhs,prhs)->running());
        }
        void cancelRemainingTasks(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->cancelRemainingTasks();
        }
        void pushTask(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->>pushTask(nrhs-1,&(prhs[1]));
        }
        void numResultOutputArgs(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateDoubleScalar(getObjectPtr(nrhs,prhs)->numResultOutputArgs());
        }
        void popResult(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->popResult().copyTo(nlhs,plhs);
        }
        void clearResults(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->clearResults();
        }
        void pause(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->pause();
        }
        void resume(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->resume();
        }
        void wasErrorThrown(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateLogicalScalar(getObjectPtr(nrhs,prhs)->wasErrorThrown());
        }
        void getError(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            std::exception_ptr err = getObjectPtr(nrhs,prhs)->getError();

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
        void clearError(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            getObjectPtr(nrhs,prhs)->clearError();
        }
    public:
        AsyncInterface(){
            using namespace std::placeholders;
            addFunction("remainingTasks",std::bind(&AsyncInterface::remainingTasks,*this,_1,_2,_3,_4));
            addFunction("availableResults",std::bind(&AsyncInterface::availableResults,*this,_1,_2,_3,_4));
            addFunction("running",std::bind(&AsyncInterface::running,*this,_1,_2,_3,_4));
            addFunction("cancelRemainingTasks",std::bind(&AsyncInterface::cancelRemainingTasks,*this,_1,_2,_3,_4));
            addFunction("pushTask",std::bind(&AsyncInterface::pushTask,*this,_1,_2,_3,_4));
            addFunction("numResultOutputArgs",std::bind(&AsyncInterface::numResultOutputArgs,*this,_1,_2,_3,_4));
            addFunction("popResult",std::bind(&AsyncInterface::popResult,*this,_1,_2,_3,_4));
            addFunction("clearResults",std::bind(&AsyncInterface::clearResults,*this,_1,_2,_3,_4));
            addFunction("pause",std::bind(&AsyncInterface::pause,*this,_1,_2,_3,_4));
            addFunction("resume",std::bind(&AsyncInterface::resume,*this,_1,_2,_3,_4));
            addFunction("wasErrorThrown",std::bind(&AsyncInterface::wasErrorThrown,*this,_1,_2,_3,_4));
            addFunction("getError",std::bind(&AsyncInterface::getError,*this,_1,_2,_3,_4));
            addFunction("clearError",std::bind(&AsyncInterface::clearError,*this,_1,_2,_3,_4));
        }
    };

}}
