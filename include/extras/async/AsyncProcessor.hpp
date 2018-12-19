#pragma once

#include <mex.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <list>

#include <extras/cmex/mxArrayGroup.hpp>

namespace extras{namespace async{

    /// Abstract class defining an Asynchronous Processor object
    class AsyncProcessor{
    protected:
        std::thread mThread;
        std::atomic<bool> ProcessAndEnd;
        std::atomic<bool> StopProcessingNow;
        std::atomic<bool> ProcessRunning;

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
                while(!ProcessAndEnd && !StopProcessingNow){
                    bool keep_proc = true;
                    while (keep_proc && !StopProcessingNow){
                        std::lock_guard<std::mutex> lock(TaskListMutex);
                        if(TaskList.size() > 0){
                            auto& task = TaskList.front();

							/*throw(runtime_error(string("in Async...:pushTask ") +
								string("nTaskArgs: ") + to_string(task.size())
							));*/

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
}}
