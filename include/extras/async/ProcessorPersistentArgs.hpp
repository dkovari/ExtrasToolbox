#pragma once

#include "AsyncProcessor.hpp"
#include <memory>
#include <utility>
#include <tuple>

namespace extras{namespace async{
    /// Async Processor with optional Persistent Arguments
    /// When the thread operates on a task the stored persistent arguments are concatenated to the task array group
    ///
    /// Usage:
    /// To add arguments call: obj.setPersistentArgs(nrhs,prhs);
    /// this will add prhs[...] to any future task arguments (created by pushTask)
    ///
    /// This class is abstract, you need to implement ProcessTask()
    template<class ResultsType=cmex::mxArrayGroup,class PushArgType=cmex::mxArrayGroup,class PersistentArgType=cmex::mxArrayGroup,class TaskType=std::pair<PushArgType,std::shared_ptr<PersistentArgType>>>
    class ProcessorWithPersistentArgs: public AsyncProcessor<ResultsType,TaskType>
    {

    protected:

        std::shared_ptr<PersistentArgType> PersistentArgs = std::make_shared<PersistentArgType>();

    public:
        /// push arguments to the task list.
        /// Each call to ProcessTask by the task thread will pop the "pushed" arguments
        /// off the stack and use them as arguments for ProcessTask(___)
        ///
        /// Note: if there are persistentArguments set at the time of pushTask()
        /// those will also be appended to the task Arguments.
        virtual void pushTask(size_t nrhs, const mxArray* prhs[])
        {

            // add task to the TaskList
            std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

            mexPrintf("\tPush pair to list\n");
            mexEvalString("pause(0.2)");

            TaskList.emplace_back(
                 std::piecewise_construct,
                 std::forward_as_tuple(nrhs,prhs),
                 std::forward_as_tuple(PersistentArgs)
            );

            mexPrintf("\tPast push pair\n");
            mexEvalString("pause(0.2)");
        }

        /// set the persistent arguments
        /// these arguments will be appended to the arguments past using pushTask
        virtual void setPersistentArgs(size_t nrhs, const mxArray* prhs[]){
            //mexPrintf("use_count: %d\nCurrentArgs =...\n",CurrentArgs.use_count());
            //mexEvalString("pause(0.2)");

            PersistentArgs = std::make_shared<cmex::mxArrayGroup>(nrhs,prhs);

            //mexPrintf("after set\nuse_count: %d\n",CurrentArgs.use_count());
            //mexEvalString("pause(0.2)");
        }

        virtual void clearPersistentArgs(){
            PersistentArgs = std::make_shared<cmex::mxArrayGroup>();
        }

    };

}}
