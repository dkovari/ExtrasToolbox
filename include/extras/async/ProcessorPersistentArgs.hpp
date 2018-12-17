#pragma once

#include "AsyncProcessor.hpp"
#include <memory>
#include <utility>

namespace extras{namespace async{
    /// Async Processor with optional Persistent Arguments
    /// When the thread operates on a task the stored persistent arguments are concatenated to the task array group
    ///
    /// Usage:
    /// To add arguments call: obj.setPersistentArgs(nrhs,prhs);
    /// this will add prhs[...] to any future task arguments (created by pushTask)
    ///
    /// This class is abstract, you need to implement ProcessTask()
    class ProcessorWithPersistentArgs: public extras::async::AsyncProcessor{
    protected:
        std::list<std::pair<cmex::mxArrayGroup,std::shared_ptr<cmex::mxArrayGroup>>> TaskList; ///< hide TaskList inherited from AsyncProcessor

        std::shared_ptr<cmex::mxArrayGroup> CurrentArgs;

        /// Method called in processing thread to execute tasks
        /// redefined here so that it uses the re-defined version of TaskList
        virtual void ProcessLoop(){
            try
            {
                while(!ProcessAndEnd && !StopProcessingNow){
                    bool keep_proc = true;
                    while (keep_proc && !StopProcessingNow){
                        std::lock_guard<std::mutex> lock(TaskListMutex);
                        if(TaskList.size() > 0){
                            auto& taskPair = TaskList.front();
                            auto& task = taskPair.first;
                            task.cat(*(taskPair.second.get()));
                            
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

    public:
        //////////
        // Task related

        /// push arguments to the task list.
        /// Each call to ProcessTask by the task thread will pop the "pushed" arguments
        /// off the stack and use them as arguments for ProcessTask(___)
        ///
        /// Note: if there are persistentArguments set at the time of pushTask()
        /// those will also be appended to the task Arguments.
        virtual void pushTask(size_t nrhs, const mxArray* prhs[])
        {
            //Convert mxArray list to array group
            cmex::mxArrayGroup AG(nrhs,prhs);

            // add task to the TaskList
            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

            TaskList.emplace_back(
                std::make_pair(
                    std::move(AG),
                    CurrentArgs
                )
            );
        }

        /// set the persistent arguments
        /// these arguments will be appended to the arguments past using pushTask
        virtual void setPersistentArgs(size_t nrhs, const mxArray* prhs[]){
            CurrentArgs = std::make_shared<cmex::mxArrayGroup>(nrhs,prhs);
        }

        virtual void clearPersistentArgs(){
            CurrentArgs = std::make_shared<cmex::mxArrayGroup>(0);
        }

    };

}}
