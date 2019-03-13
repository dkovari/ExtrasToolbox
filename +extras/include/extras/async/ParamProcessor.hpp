/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "AsyncProcessor.hpp"
#include <unordered_map>
#include <memory>
#include <extras/cmex/PersistentMxMap.hpp>

namespace extras {namespace async {

	struct TaskParamPair {
		extras::cmex::mxArrayGroup TaskArrayGroup;
		std::shared_ptr<extras::cmex::ParameterMxMap> ParameterMapPtr = std::make_shared<extras::cmex::ParameterMxMap>();

		TaskParamPair(extras::cmex::mxArrayGroup&& ArrayGroup, std::shared_ptr<extras::cmex::ParameterMxMap> Params) {
			TaskArrayGroup = std::move(ArrayGroup);
			ParameterMapPtr = Params;

			mexPrintf("Constructing TaskParamPair by ArrayGroup and shared_ptr\n");
		}
	};

	class ParamProcessor: public AsyncProcessor {
	private:
		/////////////////////////////////
		// HIDDEN INHERITED ITEMS

		cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) { throw(std::runtime_error("in ProcTask(mxAG)..shouldn't be here")); return cmex::mxArrayGroup(); } ///< don't use ProcessTask(mxArrayGroup&)
		std::list<cmex::mxArrayGroup> TaskList; //Hide original list inherited from AsyncProcessor 
	protected:
		//////////////////////////////////
		// Parameter Related

		std::shared_ptr<extras::cmex::ParameterMxMap> _pMap = std::make_shared<extras::cmex::ParameterMxMap>(); //pointer to current parameter list

		////////////////////////////////////////////////
		// Task Related

		std::mutex TaskParamListMutex;
		std::list<TaskParamPair> TaskParamList; // List containing TaskParamPair struct

		/// pure virtual method that must be defined for handling tasks
		virtual cmex::mxArrayGroup ProcessTask(const  cmex::mxArrayGroup& TaskArgs, const extras::cmex::ParameterMxMap& Params) = 0;

		/// core method called by ProcessLoop() to handle tasks.
		/// this function is responsible for getting the top element from the TaskList and calling ProcessTask
		/// it should return a bool specifying if there are more tasks to process
		/// this function is responsible for handling the TaskListMutex lock
		virtual bool ProcessLoopCore() {
			std::lock_guard<std::mutex> lock(TaskParamListMutex);
			if (TaskParamList.size() > 0) {
				auto& task = TaskParamList.front();



				cmex::mxArrayGroup res;
				if (!task.ParameterMapPtr) { // no pointer assigned
					res = ProcessTask(task.TaskArrayGroup, extras::cmex::ParameterMxMap());
				}
				else { //pass the map
					res = ProcessTask(task.TaskArrayGroup, *(task.ParameterMapPtr));
				}

				if (res.size()>0) {
					std::lock_guard<std::mutex> rlock(ResultsListMutex);
					ResultsList.push_front(std::move(res));
				}

				TaskParamList.pop_front();
			}

			if (TaskParamList.size() < 1) {
				return false;
			}
			return true;
		}


	public:
		/////////////////////////////////////
		// CTOR/DTOR

		virtual ~ParamProcessor() {
#ifdef _DEBUG
			mexPrintf("~ParamProcessor()\n");
#endif
			ProcessTasksAndEnd();
		};

		/////////////////////////////////////////////////
		// Parameter related

		// returns MxCellArray containing parameters formated as Name-Value Pairs
		virtual cmex::MxCellArray getParameters() const {
			if (!_pMap) { //uninitinalized pmap, return empty cell
				return cmex::MxCellArray();
			}
			return _pMap->map2cell();
		}

		// add or replace persistent perameters
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {
			if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParamProcessor::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}
			std::shared_ptr<extras::cmex::ParameterMxMap> newMap = std::make_shared<extras::cmex::ParameterMxMap>(); // create new, empty parameter map;
			if (_pMap) { //_pMap is not nullptr
				newMap = std::make_shared<extras::cmex::ParameterMxMap>(*_pMap); // make a copy of the parametermap
			}

			newMap->setParameters(nrhs, prhs);

			_pMap = newMap;
		}

		// clear all parameters
		virtual void clearParameters() {
			_pMap = std::make_shared<extras::cmex::ParameterMxMap>(); // create new, empty parameter map;
		}

		////////////////////////////////////////////////////
		// Task Related

		virtual void pushTask(size_t nrhs, const mxArray* prhs[])
		{
			//Convert mxArray list to array group
			cmex::mxArrayGroup AG(nrhs, prhs);
			// add task to the TaskList
			std::lock_guard<std::mutex> lock(TaskParamListMutex); //lock list
			TaskParamList.push_back(TaskParamPair(std::move(AG), _pMap));

			//// Auto start on first task
			if (_firstTask) {
				resume();
				_firstTask = false;
			}
		}

		virtual void cancelRemainingTasks() {
			mexPrintf("Trying to cancel tasks...\n");
			mexEvalString("pause(0.2)");
			StopProcessor();

			std::lock_guard<std::mutex> lock(TaskParamListMutex); //lock list
			TaskParamList.clear();

			mexPrintf("\t\t Remaining tasks cleared.\n");
			mexEvalString("pause(0.2)");
		}

		virtual size_t remainingTasks() const { return TaskParamList.size(); }

		
	};

	// Extend the AsyncInterface
	template<class ObjType, extras::SessionManager::ObjectManager<ObjType>& ObjManager> /*ObjType should be a derivative of ParamProcessor*/
	class ParamProcessorInterface :public AsyncMexInterface<ObjType, ObjManager> {
		typedef AsyncMexInterface<ObjType, ObjManager> ParentType;
	protected:
		void setParameters(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->setParameters(nrhs - 1, &(prhs[1]));
		}
		void clearParameters(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			ParentType::getObjectPtr(nrhs, prhs)->clearParameters();
		}
		void getParameters(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			plhs[0] = ParentType::getObjectPtr(nrhs, prhs)->getParameters();
		}
	public:
		ParamProcessorInterface() {
			using namespace std::placeholders;
			ParentType::addFunction("setParameters", std::bind(&ParamProcessorInterface::setParameters, this, _1, _2, _3, _4));
			ParentType::addFunction("clearParameters", std::bind(&ParamProcessorInterface::clearParameters, this, _1, _2, _3, _4));
			ParentType::addFunction("getParameters", std::bind(&ParamProcessorInterface::getParameters, this, _1, _2, _3, _4));
		}
	};

}}