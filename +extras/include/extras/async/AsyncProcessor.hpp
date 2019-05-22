/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <mex.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <list>

#include <extras/cmex/mxArrayGroup.hpp>
#include <extras/SessionManager/mexInterface.hpp>



namespace extras{namespace async{

    //! Abstract class defining an Asynchronous Processor object
    class AsyncProcessor{
    protected:

		

		/////////////////////////////////////////////////////////////
		// Task Related

		std::mutex TaskListMutex;///mutex lock for accessing TaskList
		std::list<cmex::mxArrayGroup> TaskList; //list of remaining tasks

		///Overloadable virtual method for Processing Task retrieved from the task list
		// this function is called by ProcessNextTask()
        virtual cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) = 0;

		/**Responsible for getting the top element from the TaskList and calling ProcessTask
		* MUST return cmex::mxArrayGroup
		* This function is called by ProecssLoopCore()
		* if you have redefined ProcessTask() expect different arguments, then you need to also redefine this method
		*/
		virtual cmex::mxArrayGroup ProcessNextTask() {
			cmex::mxArrayGroup out; //init empty array group;
			std::lock_guard<std::mutex> lock(TaskListMutex); //lock the task list. If you are using a different lock mechanism change this

			if (remainingTasks() > 0) {
				auto& task = TaskList.front(); //if you are using a custom TaskList, change this code

				out = ProcessTask(task); //if processTask is redefined, change this too!

				TaskList.pop_front();
			}

			return out;
		}

		/** Core method called by ProcessLoop() to handle tasks.
		* This function is responsible for calling ProcessNextTask()
		* it should return a bool specifying if there are more tasks to process
		* 
		* Unless you know what you are doing, it is not a good idea to redefine this method (even though it is virtual)
		*/
		virtual bool ProcessLoopCore() {

			if (remainingTasks() > 0) {
				auto results = ProcessNextTask();

				// store result on results list
				std::lock_guard<std::mutex> rlock(ResultsListMutex);

				if (results.size() > 0) {

					ResultsList.push_front(std::move(results));

				}
			}

			if (remainingTasks() < 1) {
				return false;
			}

			return true;
		}

        /** Method called in processing thread to execute tasks
		* Calls ProceesLoopCore() and catches any errors thrown
		* You probably don't need to overload this function
		*
		* Unless you know what you are doing, it is not a good idea to redefine this method (even though it is virtual)
		*/
        virtual void ProcessLoop(){
			using namespace std;
            try
            {
				while(!ProcessAndEnd && !StopProcessingNow){ //enter thread loop, unless thread is stopped and restared, we won't re-enter this loop
                    bool keep_proc = true;
                    while (keep_proc && !StopProcessingNow){
						keep_proc = ProcessLoopCore();
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

		///////////////////////////////////////////
		// Results related

		std::mutex ResultsListMutex; ///mutex lock for accessing ResultsList
		std::list<cmex::mxArrayGroup> ResultsList; ///results list

		/////////////////////////////////////////
		// Thread control related

		std::thread mThread;
		std::atomic<bool> ProcessAndEnd; //flag specifying that all remaining processes should be run
		std::atomic<bool> StopProcessingNow; //flag specifying that thread should stop immediately
		std::atomic<bool> ProcessRunning; //flag if thread is running

		bool _firstTask = true;

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
				mexPrintf("\tWaiting for AsyncProcessor to finish");
				mexEvalString("pause(0.2)");
				mThread.join();
                mexPrintf("...done.\n");
                mexEvalString("pause(0.1)");
			}
            ProcessRunning = false;
        }
        virtual void StopProcessor(){
            StopProcessingNow = true;
            std::this_thread::sleep_for(std::chrono::microseconds(1000)); //let some time pass
            if (mThread.joinable()) {
				mexPrintf("\tHalting AsyncProcessor");
				mexEvalString("pause(0.2)");
				mThread.join();
                mexPrintf("...done.\n");
                mexEvalString("pause(0.1)");
			}
            ProcessRunning = false;
        }

		////////////////////////////////////////////////////////
		// Error related (also used in thead control methods)

		std::atomic<bool> ErrorFlag;
		std::mutex LastErrorMutex;
		std::exception_ptr LastError = nullptr;

    public:
        //! Destructor
        virtual ~AsyncProcessor(){
#ifdef _DEBUG
			mexPrintf("~AsyncProcessor()\n");
#endif
            ProcessTasksAndEnd();
        }

        //! Constructor
        AsyncProcessor(){
            ProcessRunning = false;

			// DON'T START PROCESSOR UNTIL FIRST TASK IS ADDED
			_firstTask = true;
            //StartProcessor();
        }

		//////////////////////////////////////////////////
		// Thread control related

        void pause() {
            StopProcessor();
        }

        void resume(){
            if(!ProcessRunning){
                StartProcessor();
            }
        }

		bool running() const { return ProcessRunning; }

		////////////////////////////////////////////////////
		// Task Related

		virtual void pushTask(size_t nrhs, const mxArray* prhs[])
		{
			//Convert mxArray list to array group
			cmex::mxArrayGroup AG(nrhs, prhs);

			// add task to the TaskList
			std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

			TaskList.push_back(std::move(AG)); //move task to back of task list

			///! Auto start on first task
			if (_firstTask) {
				resume();
				_firstTask = false;
			}
		}

        virtual void cancelRemainingTasks(){
#ifdef _DEBUG
			mexPrintf("Trying to cancel tasks...\n");
			mexEvalString("pause(0.2)");
#endif
            StopProcessor();

            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list
            TaskList.clear();
#ifdef _DEBUG
            mexPrintf("\t\t Remaining tasks cleared.\n");
            mexEvalString("pause(0.2)");
#endif
        }

        virtual size_t remainingTasks() const {return TaskList.size();}

        /////////////////////////////////////////////////////////
		// Results Related

		size_t availableResults() const { return ResultsList.size(); }

        virtual cmex::mxArrayGroup popResult(){
            if(ResultsList.size()<1){
                throw(extras::stacktrace_error("No results in the ResultsList, cannot popResult()"));
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

        //! number of results output arguments for next result
        virtual size_t numResultOutputArgs() const{
            if(ResultsList.size()<1){
                return 0;
            }else{
                return ResultsList.back().size();
            }
        }

		/////////////////////////////////////////////
		// Error Related

        //! true if there was an error
        virtual bool wasErrorThrown() const{
            return ErrorFlag;
        }

        //! Clears the error pointer and error flag.
        //! After calling ErroFlag=false and LastError=nullptr
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

    //! Implement mexInterface for AsyncProcessor
    template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of AsyncProcessor*/
    class AsyncMexInterface: public SessionManager::mexInterface<ObjType, ObjManager>{
        typedef SessionManager::mexInterface<ObjType, ObjManager> ParentType;
    protected:
        void remainingTasks(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateDoubleScalar((double)(ParentType::getObjectPtr(nrhs,prhs)->remainingTasks()));
        }
        void availableResults(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateDoubleScalar((double)(ParentType::getObjectPtr(nrhs,prhs)->availableResults()));
        }
        void running(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs,prhs)->running());
        }
        void cancelRemainingTasks(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            ParentType::getObjectPtr(nrhs,prhs)->cancelRemainingTasks();
        }
        void pushTask(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            ParentType::getObjectPtr(nrhs,prhs)->pushTask(nrhs-1,&(prhs[1]));
        }
        void numResultOutputArgs(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateDoubleScalar((double)(ParentType::getObjectPtr(nrhs,prhs)->numResultOutputArgs()));
        }
        void popResult(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            ParentType::getObjectPtr(nrhs,prhs)->popResult().copyTo(nlhs,plhs);
        }
        void clearResults(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            ParentType::getObjectPtr(nrhs,prhs)->clearResults();
        }
        void pause(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            ParentType::getObjectPtr(nrhs,prhs)->pause();
        }
        void resume(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            ParentType::getObjectPtr(nrhs,prhs)->resume();
        }
        void wasErrorThrown(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            plhs[0] = mxCreateLogicalScalar(ParentType::getObjectPtr(nrhs,prhs)->wasErrorThrown());
        }
        void getError(int nlhs,mxArray* plhs[],int nrhs, const mxArray* prhs[]){
            std::exception_ptr err = ParentType::getObjectPtr(nrhs,prhs)->getError();

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
            ParentType::getObjectPtr(nrhs,prhs)->clearError();
        }

    public:
        AsyncMexInterface(){
            using namespace std::placeholders;
            ParentType::addFunction("remainingTasks",std::bind(&AsyncMexInterface::remainingTasks,this,_1,_2,_3,_4));
            ParentType::addFunction("availableResults",std::bind(&AsyncMexInterface::availableResults,this,_1,_2,_3,_4));
            ParentType::addFunction("running",std::bind(&AsyncMexInterface::running,this,_1,_2,_3,_4));
            ParentType::addFunction("cancelRemainingTasks",std::bind(&AsyncMexInterface::cancelRemainingTasks,this,_1,_2,_3,_4));
            ParentType::addFunction("pushTask",std::bind(&AsyncMexInterface::pushTask,this,_1,_2,_3,_4));
            ParentType::addFunction("numResultOutputArgs",std::bind(&AsyncMexInterface::numResultOutputArgs,this,_1,_2,_3,_4));
            ParentType::addFunction("popResult",std::bind(&AsyncMexInterface::popResult,this,_1,_2,_3,_4));
            ParentType::addFunction("clearResults",std::bind(&AsyncMexInterface::clearResults,this,_1,_2,_3,_4));
            ParentType::addFunction("pause",std::bind(&AsyncMexInterface::pause,this,_1,_2,_3,_4));
            ParentType::addFunction("resume",std::bind(&AsyncMexInterface::resume,this,_1,_2,_3,_4));
            ParentType::addFunction("wasErrorThrown",std::bind(&AsyncMexInterface::wasErrorThrown,this,_1,_2,_3,_4));
            ParentType::addFunction("getError",std::bind(&AsyncMexInterface::getError,this,_1,_2,_3,_4));
            ParentType::addFunction("clearError",std::bind(&AsyncMexInterface::clearError,this,_1,_2,_3,_4));
		}
    };

}}
