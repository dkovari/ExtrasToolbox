#pragma once

#include <mex.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <list>

#include "mxArrayGroup.h"

namespace mex{
    class AsyncProcessor{
    protected:
        std::thread mThread;
        std::atomic<bool> ProcessAndEnd;
        std::atomic<bool> StopProcessingNow;
        std::atomic<bool> ProcessRunning;

        std::atomic<bool> ErrorFlag;
        std::exception_ptr LastError;

        std::mutex TaskListMutex;///mutex lock for accessing TaskList
        std::list<mxArrayGroup> TaskList; //list of remaining tasks

        std::mutex ResultsListMutex; ///mutex lock for accessing ResultsList
        std::list<mxArrayGroup> ResultsList; ///results list

		///Overloadable virtual method for Processing Tasks in the task list
        virtual mxArrayGroup ProcessTask(const mxArrayGroup& args) = 0;

        /// Method called in processing thread to execute tasks
        virtual void ProcessLoop(){
            try
            {
                while(!ProcessAndEnd && !StopProcessingNow){
                    bool keep_proc = true;
                    while (keep_proc && !StopProcessingNow){
                        std::lock_guard<std::mutex> lock(TaskListMutex);
                        if(TaskList.size() > 0){
                            auto& task = TaskList.front();
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
                LastError = std::current_exception();
                ErrorFlag = true;
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

        void cancelRemainingTasks(){
            StopProcessor();
            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list
            TaskList.clear();
        }

        size_t remainingTasks() const {return TaskList.size();}
        size_t availableResults() const {return ResultsList.size();}
        bool running() const {return ProcessRunning;}

        virtual void pushTask(size_t nrhs, const mxArray* prhs[])
        {
            //Convert mxArray list to array group
            mxArrayGroup AG(nrhs,prhs);

            // add task to the TaskList
            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

            TaskList.emplace_back(std::move(AG)); //move task to back of task list
        }

        virtual mxArrayGroup popResult(){
            if(ResultsList.size()<1){
                throw(std::runtime_error("No results in the ResultsList, cannot popResult()"));
            }

            //pop results
            std::lock_guard<std::mutex> lock(ResultsListMutex); //lock list
			mxArrayGroup out = std::move(ResultsList.back());
            ResultsList.pop_back();
            return out;
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
}
