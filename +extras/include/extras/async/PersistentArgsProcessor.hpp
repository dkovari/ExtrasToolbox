/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "AsyncProcessor.hpp"
#include <memory>
#include <utility>
#include <extras/cmex/MxCellArray.hpp>

namespace extras{namespace async{

    /// Async Processor with optional Persistent Arguments
    /// When the thread operates on a task the stored persistent arguments are concatenated to the task array group
    ///
    /// Usage:
    /// To add arguments call: obj.setPersistentArgs(nrhs,prhs);
    /// this will add prhs[...] to any future task arguments (created by pushTask)
    ///
    /// This class is abstract, you need to implement ProcessTask()
	template<class PersistentArgType=cmex::mxArrayGroup>
    class PersistentArgsProcessor: public extras::async::AsyncProcessor{
	public:
		typedef typename std::pair<extras::cmex::mxArrayGroup,std::shared_ptr<PersistentArgType>> TaskPairType;
		typedef typename std::shared_ptr<PersistentArgType> PersistentArg_Ptr;
	private:
		cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) { throw(std::runtime_error("in ProcTask(mxAG)..shouldn't be here")); return cmex::mxArrayGroup(); } ///< don't use ProcessTask(mxArrayGroup&)
    protected:
        std::list<TaskPairType> TaskList; // Hides TaskList inherited from AsyncProcessor

        PersistentArg_Ptr CurrentArgs = std::make_shared<PersistentArgType>();

        //virtual cmex::mxArrayGroup ProcessTask(const TaskPairType&) = 0; ///< must define ProcessTask for working with pushed task and persistent args
		virtual cmex::mxArrayGroup ProcessTask(const TaskPairType&) = 0;

		/// core method called by ProcessLoop() to handle tasks.
		/// this function is responsible for getting the top element from the TaskList and calling ProcessTask
		/// it should return a bool specifying if there are more tasks to process
		/// this function is responsible for handling the TaskListMutex lock
		///
		/// Redefined here to make sure this version of TaskList is used
		virtual bool ProcessLoopCore() {
			std::lock_guard<std::mutex> lock(TaskListMutex);
			if (TaskList.size() > 0) {
				auto& taskPair = TaskList.front();

				//DO Task
				auto res = ProcessTask(taskPair);
				std::lock_guard<std::mutex> rlock(ResultsListMutex);

				if (res.size()>0) {
					ResultsList.push_front(std::move(res));
				}

				TaskList.pop_front();
			}

			if (TaskList.size() < 1) {
				return false;
			}
			return true;
		}

    public:

        /// push arguments to the task list.
        /// Each call to ProcessTask by the task thread will pop the "pushed" arguments
        /// off the stack and use them as arguments for ProcessTask(___)
        ///
        /// Note: if there are persistentArguments set at the time of pushTask()
        /// those will also be appended to the task Arguments.
        virtual void pushTask(size_t nrhs, const mxArray* prhs[])
        {
			using namespace std;
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

		// redefine to force use of new TaskList
		virtual void cancelRemainingTasks() {
			StopProcessor();

			std::lock_guard<std::mutex> lock(TaskListMutex); //lock list
			TaskList.clear();

			mexPrintf("\t\t Remaining tasks cleared.\n");
			mexEvalString("pause(0.2)");
		}

		// redefine to force use of new TaskList
		virtual size_t remainingTasks() const { return TaskList.size(); }

        /// set the persistent arguments
        /// these arguments will be appended to the arguments past using pushTask
		virtual void setPersistentArgs(size_t nrhs, const mxArray* prhs[]);

        virtual void clearPersistentArgs(){
            CurrentArgs = std::make_shared<PersistentArgType>();
        }

		/// Get current values of the persistent args
		/// Note: this method is abstract, however if you want to derive a class using
		/// PersistentArgType=cmex::mxArrayGroup
		/// you can use the default implementation of getPersistentArgs() included in this header
		/// Example:
		///		class YourClass: public PersistentArgsProcessor<>{
		///			...
		///			extras::cmex::MxCellArray getPersistentArgs() const{
		///				return PersistentArgsProcessor<>::getPersistentArgs();
		///			}
		///		};
		virtual extras::cmex::MxCellArray getPersistentArgs() const;

	public:
		virtual ~PersistentArgsProcessor() {};

    };

	/// Implement getPersistentArgs for mxArrayGroup type args
	///	PersistentArgsProcessor<...>::getPersistentArgs() is abstract, however you can use this implementation in you derived classes
	/// Example:
	///		class YourClass: public PersistentArgsProcessor<>{
	///			...
	///			extras::cmex::MxCellArray getPersistentArgs() const{
	///				return PersistentArgsProcessor<>::getPersistentArgs();
	///			}
	///		};
	template<> extras::cmex::MxCellArray PersistentArgsProcessor<cmex::mxArrayGroup>::getPersistentArgs() const {
		extras::cmex::MxCellArray out;
		out.reshape(1, CurrentArgs->size());
		/// copy args to output cell array
		for (size_t n = 0; n < CurrentArgs->size(); ++n) {
			out(n) = CurrentArgs->getConstArray(n);
		}
		return out;
	}

	// default setPersistentArg method
	template<> void PersistentArgsProcessor<cmex::mxArrayGroup>::setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {
		CurrentArgs = std::make_shared<cmex::mxArrayGroup>(nrhs,prhs);
	}

	// Extend the AsyncInterface
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of PersistentArgsProcessor*/
	class PersistentArgsProcessorInterface :public AsyncMexInterface<ObjType, ObjManager> {
		typedef AsyncMexInterface<ObjType, ObjManager> ParentType;
	protected:
		void setPersistentArgs(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->setPersistentArgs(nrhs - 1, &(prhs[1]));
		}
		void clearPersistentArgs(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearPersistentArgs();
		}
		void getPersistentArgs(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = ParentType::getObjectPtr(nrhs, prhs)->getPersistentArgs();
		}
	public:
		PersistentArgsProcessorInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("setPersistentArgs", std::bind(&PersistentArgsProcessorInterface::setPersistentArgs, this, _1, _2, _3, _4));
			ParentType::addFunction("clearPersistentArgs", std::bind(&PersistentArgsProcessorInterface::clearPersistentArgs, this, _1, _2, _3, _4));
			ParentType::addFunction("getPersistentArgs", std::bind(&PersistentArgsProcessorInterface::getPersistentArgs, this, _1, _2, _3, _4));
		}
	};


}}
