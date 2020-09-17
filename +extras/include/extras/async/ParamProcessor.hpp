/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
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
		}
	};

	class ParamProcessor: public AsyncProcessorWithWriter {
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
		virtual cmex::mxArrayGroup ProcessTask(const  cmex::mxArrayGroup& TaskArgs, std::shared_ptr<const extras::cmex::ParameterMxMap> MapPtr) = 0;


		/**Responsible for getting the top element from the TaskList and calling ProcessTask
		* MUST return cmex::mxArrayGroup
		* This function is called by ProecssLoopCore()
		* Redefined here to accomodate  ProcessTask(const  cmex::mxArrayGroup& TaskArgs, std::shared_ptr<const extras::cmex::ParameterMxMap> MapPtr)
		* Uses TaskParamListMutex and TaskParamList instead of TaskList
		*/
		virtual cmex::mxArrayGroup ProcessNextTask() {
			cmex::mxArrayGroup out; //init empty array group;
			std::lock_guard<std::mutex> lock(TaskParamListMutex); //lock the task list. If you are using a different lock mechanism change this

			if (remainingTasks() > 0) {
				auto& task = TaskParamList.front(); //if you are using a custom TaskList, change this code

				out = ProcessTask(task.TaskArrayGroup, task.ParameterMapPtr); //if processTask is redefined, change this too!

				TaskParamList.pop_front();
			}

			return out;
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

		// returns MxObject containing parameters formated as a struct
		virtual cmex::MxObject getParameters() const {
			if (!_pMap) { //uninitinalized pmap, return empty cell
				return cmex::MxStruct();
			}
			return _pMap->map2struct();
		}

		// add or replace persistent perameters
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {
			/*if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParamProcessor::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}*/
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
	class ParamProcessorInterface :public AsyncProcessorWithWriterInterface<ObjType, ObjManager> {
		typedef AsyncProcessorWithWriterInterface<ObjType, ObjManager> ParentType;
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