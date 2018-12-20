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
	template<class PersistentArgType=cmex::mxArrayGroup>
    class PersistentArgsProcessor: public extras::async::AsyncProcessor{
	public:
		typedef std::pair<extras::cmex::mxArrayGroup,std::shared_ptr<PersistentArgType>> TaskPairType;
	private:
		cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) { throw(std::runtime_error("in ProcTask(mxAG)..shouldn't be here")); return cmex::mxArrayGroup(); } ///< don't use ProcessTask(mxArrayGroup&)
    protected:
        std::list<TaskPairType> TaskList; // Hides TaskList inherited from AsyncProcessor

        std::shared_ptr<PersistentArgType> CurrentArgs = std::make_shared<PersistentArgType>();

        virtual cmex::mxArrayGroup ProcessTask(const TaskPairType&) = 0; ///< must define ProcessTask for working with pushed task and persistent args

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
#ifdef _DEBUG
            mexPrintf("\tPush pair to list\n");
            mexEvalString("pause(0.2)");
#endif
            TaskList.emplace_back(
                std::make_pair(
                    std::move(AG),
                    CurrentArgs
                )
            );
#ifdef _DEBUG
            mexPrintf("\tPast push pair\n");
            mexEvalString("pause(0.2)");
#endif

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

#ifdef _DEBUG
	public:
		virtual ~PersistentArgsProcessor() {
			mexPrintf("~PersistentArgsProcessor()\n");
		}
#endif

    };

	// default setPersistentArg method
	void PersistentArgsProcessor<cmex::mxArrayGroup>::setPersistentArgs(size_t nrhs, const mxArray* prhs[]) {
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
	public:
		PersistentArgsProcessorInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("setPersistentArgs", std::bind(&PersistentArgsProcessorInterface::setPersistentArgs, *this, _1, _2, _3, _4));
			ParentType::addFunction("clearPersistentArgs", std::bind(&PersistentArgsProcessorInterface::clearPersistentArgs, *this, _1, _2, _3, _4));
		}
	};


}}
