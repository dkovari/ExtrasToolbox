/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

/********************************************************************
COMPRESSION Includes
=====================================================================
This header depends on ZLIB for reading and writing files compressed
with the gz format. Therefore you need to have zlib built/installed
on your system.

A version of ZLIB is included with the ExtrasToolbox located in
.../+extras/external_libs/zlib
Look at that folder for build instructions.
Alternatively, is you are using a *nix-type system, you might have
better luck using your package manager to install zlib.

When building, be sure to include the location of zlib.h and to link to the
compiled zlib-lib files.
*********************************************/
#include "AsyncProcessorWithWriter.hpp"
/********************************************/

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
    class PersistentArgsProcessor: public extras::async::AsyncProcessorWithWriter {
	public:
		typedef typename std::pair<extras::cmex::mxArrayGroup,std::shared_ptr<PersistentArgType>> TaskPairType;
		typedef typename std::shared_ptr<PersistentArgType> PersistentArg_Ptr;
	private:
		cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) { throw(extras::stacktrace_error("in ProcTask(mxAG)..shouldn't be here")); return cmex::mxArrayGroup(); } ///< don't use ProcessTask(mxArrayGroup&)
    protected:
        std::list<TaskPairType> TaskList; // Hides TaskList inherited from AsyncProcessor

        PersistentArg_Ptr CurrentArgs = std::make_shared<PersistentArgType>();

        //virtual cmex::mxArrayGroup ProcessTask(const TaskPairType&) = 0; ///< must define ProcessTask for working with pushed task and persistent args
		virtual cmex::mxArrayGroup ProcessTask(const TaskPairType&) = 0;

		/**Responsible for getting the top element from the TaskList and calling ProcessTask
		* MUST return cmex::mxArrayGroup
		* This function is called by ProecssLoopCore()
		*
		* Redefined here to make sure this version of TaskList is used
		*/
		virtual cmex::mxArrayGroup ProcessNextTask() {
			cmex::mxArrayGroup out; //init empty array group;
			std::lock_guard<std::mutex> lock(TaskListMutex); //lock the task list. If you are using a different lock mechanism change this

			if (remainingTasks() > 0) {
				auto& taskPair = TaskList.front(); //if you are using a custom TaskList, change this code

				out = ProcessTask(taskPair);

				TaskList.pop_front();
			}
			return out;
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

            TaskList.push_back(
                std::make_pair(
                    std::move(AG),
                    CurrentArgs
                )
            );

			//// Auto start on first task
			if (_firstTask) {
				resume();
				_firstTask = false;
			}
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
		virtual ~PersistentArgsProcessor() {
#ifdef _DEBUG
			mexPrintf("~PersistentArgsProcessor()\n");
#endif
			ProcessTasksAndEnd();
		};

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
	class PersistentArgsProcessorInterface :public AsyncProcessorWithWriterInterface<ObjType, ObjManager> {
		typedef AsyncProcessorWithWriterInterface<ObjType, ObjManager> ParentType;
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
