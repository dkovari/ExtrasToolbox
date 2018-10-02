#pragma once

#include <mex.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <list>
#include <vector>

#include <mxobject.hpp>

namespace mex{
    class AsyncProcessor{
    protected:
        std::thread mThread;
        std::atomic<bool> ProcessAndEnd;
        std::atomic<bool> StopProcessingNow;
        std::atomic<bool> ProcessRunning;

        std::atomic<bool> ErrorFlag;
        std::exception_ptr LastError;

        std::mutex TaskListMutex;//mutex lock for accessing TaskList
        std::list<std::vector<MxObject>> TaskList; 

        std::mutex ResultsListMutex; //mutex lock for accessing ResultsList
        std::list<std::vector<MxObject>> ResultsList;

		//Overloadable virtual method for Processing Tasks in the task list
		//Note: remember to make the MxObjects returned by the method persistent
        virtual std::vector<MxObject> ProcessTask(const std::vector<MxObject>& Args){return std::vector<MxObject>();}

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

        void pauseProcessor() {
            StopProcessor();
        }

        void resumeProcessor(){
            if(!ProcessRunning){
                StartProcessor();
            }
        }

        size_t remainingTasks() const {return TaskList.size();}
        size_t availableResults() const {return ResultsList.size();}
        bool running() const {return ProcessRunning;}

        virtual void pushTask(size_t nrhs, const mxArray* prhs[])
        {
            // add task to the TaskList
            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

			std::vector<MxObject> tsk;
			//copy rhs data to stack
			tsk.reserve(nrhs);
			for (size_t n = 0; n < nrhs; ++n) {
				tsk.emplace_back(prhs[n]);
				tsk.back().makePersistent(); //make array persistent so we don't get memory errors
			}

            TaskList.emplace_back( std::move(tsk)); //move task to back of task list
        }

        virtual std::vector<MxObject> popResult(){
            if(ResultsList.size()<1){
                throw(std::runtime_error("No results in the ResultsList, cannot popResult()"));
            }

            //pop results
            std::lock_guard<std::mutex> lock(ResultsListMutex); //lock list
			std::vector<MxObject> out = std::move(ResultsList.back()) ;
            ResultsList.pop_back();
            return out;
        }

        virtual size_t numResultOutputArgs() const{
            if(ResultsList.size()<1){
                return 0;
            }else{
                return ResultsList.back().size();
            }
        }

        virtual bool errorThrown() const{
            return ErrorFlag;
        }

        //check for errors thrown within the processing loop
		//returns exception_ptr to the thrown exception
		//if there aren't any exceptions, returns nullptr
		virtual std::exception_ptr checkError() const {
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
