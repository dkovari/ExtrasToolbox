/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include "AsyncProcessor.hpp"
#include <unordered_map>
#include <memory>
#include <extras/cmex/PersistentMxArray.hpp>
#include <extras/cmex/MxCellArray.hpp>
#include <extras/cmex/mexextras.hpp>

namespace extras {namespace async {

	class ParamProcessor: public AsyncProcessor {
	public:
		typedef typename cmex::persistentMxArray ParameterType;
		typedef typename std::shared_ptr<ParameterType> ParameterPtr;
		typedef typename std::unordered_map<std::string, ParameterPtr> ParameterMap;
		typedef typename std::shared_ptr<ParameterMap> ParameterMapPtr;
		typedef typename std::pair<extras::cmex::mxArrayGroup, ParameterMapPtr> TaskParameterPair;
	private:
		cmex::mxArrayGroup ProcessTask(const cmex::mxArrayGroup& args) { throw(std::runtime_error("in ProcTask(mxAG)..shouldn't be here")); return cmex::mxArrayGroup(); } ///< don't use ProcessTask(mxArrayGroup&)
	protected:
		ParameterMapPtr _pMap = std::make_shared<ParameterMap>(); //pointer to current parameter list

		std::list<TaskParameterPair> TaskList; // Hides TaskList inherited from AsyncProcessor

		/// pure virtual method that must be defined for handling tasks
		virtual cmex::mxArrayGroup ProcessTask(const  cmex::mxArrayGroup& TaskArgs, const ParameterMap& Params) = 0;

		/// core method called by ProcessLoop() to handle tasks.
		/// this function is responsible for getting the top element from the TaskList and calling ProcessTask
		/// it should return a bool specifying if there are more tasks to process
		/// this function is responsible for handling the TaskListMutex lock
		///
		/// Redefined here to make sure this version of TaskList is used
		virtual bool ProcessLoopCore() {
			std::lock_guard<std::mutex> lock(TaskListMutex);
			if (!TaskList.empty()) {
				auto& taskPair = TaskList.front();

				//DO Task
				cmex::mxArrayGroup res;
				if (!(taskPair.second)) { //param list is empty
					res = ProcessTask(taskPair.first, ParameterMap());
				}
				else {
					res = ProcessTask(taskPair.first, *(taskPair.second));
				}
				std::lock_guard<std::mutex> rlock(ResultsListMutex);

				if (res.size()>0) {
					ResultsList.push_front(std::move(res));
				}

				TaskList.pop_front();
			}

			if (TaskList.empty()) {
				return false;
			}
			return true;
		}

		/// convert ParameterMap to mxArrayGroup
		/// could be useful for passing parameter args to functions designed with MATLAB mexFunction(...) interface
		/// NOTE: This will create a copy of the Data stored in each parameter. Consequently it could be slow if the data is large.
		static cmex::mxArrayGroup map2arraygroup(const ParameterMap& map) {
			extras::cmex::mxArrayGroup out(2 * map.size());
			size_t k = 0;
			for (const auto& p : map) {
				out.setArray(k, extras::cmex::MxObject(p.first).getmxarray());
				out.setArray(k + 1, p.second->getMxArray());
				k += 2;
			}
		}

	public:
		/// alias of  std::make_shared<cmex::persistentMxArray>(src)
		static ParameterPtr makeParameterPtr(const ParameterType& src){
			return std::make_shared<ParameterType>(src);
		}

		/// alias of std::make_shared<cmex::persistentMxArray>(src)
		static ParameterPtr makeParameterPtr(ParameterType&& src) {
			return std::make_shared<ParameterType>(std::move(src));
		}

		virtual ~ParamProcessor() {};

		/// returns MxCellArray containing parameters formated as Name-Value Pairs
		virtual cmex::MxCellArray getParameters() const {
			cmex::MxCellArray out;

			if (!_pMap) { //_pMap has not been set (i.e. nullptr)
				return out; //return empty cell
			}
			
			if (_pMap->empty()) { //_pMap is empty
				return out;
			}

			out.reshape({ 2,_pMap->size() });
			size_t k = 0;
			for (const auto& param : *_pMap) {
				out({ 0,k }) = param.first.c_str(); //set first cell param name
				out({ 1,k }) = param.second->getMxArray(); //set second cell to copy of param mxArray
				k++;
			}

			return out;
		}

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
			cmex::mxArrayGroup AG(nrhs, prhs);

			// add task to the TaskList
			std::lock_guard<std::mutex> lock(TaskListMutex); //lock list

			TaskList.emplace_back(
				std::make_pair(
					std::move(AG),
					_pMap
				)
			);

			auto& taskPair = TaskList.front();
			mexPrintf("front args size: %d,%d\n", taskPair.first.size(),taskPair.second->size());
			mexEvalString("pause(0.2)");
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

		/// add or replace persistent perameters
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {
			if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParamProcessor::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}
			ParameterMapPtr newMap = std::make_shared<ParameterMap>(); // create new, empty parameter map;
			if (_pMap) { //_pMap is not nullptr
				newMap = std::make_shared<ParameterMap>(*_pMap); // make a copy of the parametermap
			}

			// loop over args and set parameters
			for (size_t n = 0; n < nrhs - 1; n += 2) {
				(*newMap)[cmex::getstring(prhs[n])] = makeParameterPtr(prhs[n+1]);
			}

			_pMap = newMap;
		}

		/// clear all parameters
		virtual void clearParameters() {
			_pMap = std::make_shared<ParameterMap>(); // create new, empty parameter map;
		}
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