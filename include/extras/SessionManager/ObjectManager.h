/*
Mex framework for creating C++ object which live past the end of each mex call,
allowing the object to be used across multiple calls.

Couple this the mexDispatch to create a class interface

This is intended to be used with Session.m
*/

#pragma once

#include <unordered_map>
#include <memory>
#include <mex.h>
#include <functional>

namespace extras{namespace SessionManager{


	template <class Obj>
	class ObjectManager {
	protected:
		bool _lock_mex;
		std::unordered_map<intptr_t, std::shared_ptr<Obj>> ObjectMap;

		

		static intptr_t getIntPointer(const mxArray* pointer) {
			if (mxIsEmpty(pointer))
				throw(std::runtime_error("ObjectManager:invalidType -> Id is empty."));
			if (sizeof(intptr_t) == 8 && !mxIsInt64(pointer) && !mxIsUint64(pointer))
				throw(std::runtime_error("ObjectManager:invalidType -> Invalid ID type, pointer ID must be INT64 or UINT64."));
			if (sizeof(intptr_t) == 4 && !mxIsInt32(pointer) && !mxIsUint32(pointer))
				throw(std::runtime_error("ObjectManager:invalidType -> Invalid ID type, pointer ID must be INT32 or UINT32."));
			return *reinterpret_cast<intptr_t*>(mxGetData(pointer));
		}

		void clearObjects() {
			for (auto it = ObjectMap.begin(); it != ObjectMap.end();) {
				mexPrintf("id: %d\n", it->first);
				mexEvalString("pause(0.2)");
				destroy(it->first);
			}
		}

	public:
		static std::list < ObjectManager<Obj>*> OM_list;
		static bool exitSet;

		static void exitFn() {
#ifdef _DEBUG
			mexPrintf("in exitFn()\n");
			mexPrintf("Clearing mexFunction: %s\n", mexFunctionName());
			mexEvalString("pause(0.2)");
#endif
			for (auto pOB : OM_list) {
				pOB->clearObjects();
			}
		}


		ObjectManager(bool LOCK_MEX = true){
			_lock_mex = LOCK_MEX;
#ifdef _DEBUG
			mexPrintf("Creating ObjectManager<%s>\n", typeid(Obj).name());
			if (_lock_mex) {
				mexPrintf("\tLocking mex file: %s\n", mexFunctionName());
				mexEvalString("pause(0.2)");
			}
#endif
			if (!_lock_mex) {
#ifdef _DEBUG
				mexPrintf("registering with OM_list\n");
				mexEvalString("pause(0.2)");
#endif
#ifdef _DEBUG
				mexPrintf("OM_list.size(): %d\n", OM_list.size());
				mexEvalString("pause(0.2)");
#endif
				// register with static OM_list
				OM_list.push_back(this);

#ifdef _DEBUG
				mexPrintf("OM_list.size(): %d\n",OM_list.size());
				mexEvalString("pause(0.2)");
#endif

				if (!exitSet) {
#ifdef _DEBUG
					mexPrintf("set mexAtExit(...)\n");
					mexEvalString("pause(0.2)");
#endif
					mexAtExit(&ObjectManager::exitFn);
					exitSet = true;
				}
			}
		}

		~ObjectManager(){
#ifdef _DEBUG
			mexPrintf("Destroying ObjectManager<%s>\n",typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif
			clearObjects();

			for (auto it = OM_list.begin(); it != OM_list.end();) { //deregister from list
				if (*it == this) {
					OM_list.erase(it);
				}
				else {
					++it;
				}
			}

		}

		//Add object to map, creates a shared_ptr from the pointer
		// call using something like objman.create(new YourObj());
		intptr_t create(Obj* p) { 
#ifdef _DEBUG
			mexPrintf("ObjectManager<%s>::create\n", typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif
			std::shared_ptr<Obj> newObj(p);
			intptr_t ptr = reinterpret_cast<intptr_t>(p);
			ObjectMap.insert(std::make_pair(ptr, newObj));//add object to map

			if (_lock_mex) {
				mexLock(); //increment mex lock counter;
			}
			

			return ptr;
		}

		//destroy instance
		void destroy(intptr_t id) {
#ifdef _DEBUG
			mexPrintf("ObjectManager<%s>::destroy(intptr_t id)\n", typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif
			if (!ObjectMap.empty())
			{
				ObjectMap.erase(id);
				if (_lock_mex) { //unlock mex file if needed
					mexUnlock(); //deccrement mex lock counter;
				}
			}
		}
		void destroy(const mxArray* in) {
			intptr_t id = getIntPointer(in);
			destroy(id);
		}

		std::shared_ptr<Obj> get(intptr_t id) {
#ifdef _DEBUG
			mexPrintf("ObjectManager<%s>::get()\n\tnObjects=%d\n", typeid(Obj).name(),ObjectMap.size());
			mexEvalString("pause(0.2)");
#endif
			auto search = ObjectMap.find(id);
			if (search != ObjectMap.end()) {
				return search->second;
			}
			else {
				throw(std::runtime_error("ObjectManager::get() -> Object not found"));
			}
		}

		std::shared_ptr<Obj> get(const mxArray* in) {
			intptr_t id = getIntPointer(in);
#ifdef _DEBUG
			mexPrintf("\t ptr=%d\n", typeid(Obj).name(),id);
			mexEvalString("pause(0.2)");
#endif
			return get(id);
		}
	};
}}
template <class Obj> std::list < extras::SessionManager::ObjectManager<Obj>*> extras::SessionManager::ObjectManager<Obj>::OM_list = std::list < extras::SessionManager::ObjectManager<Obj>*>();
template <class Obj> bool extras::SessionManager::ObjectManager<Obj>::exitSet = false;
