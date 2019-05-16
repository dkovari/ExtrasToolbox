/*
Mex framework for creating C++ object which live past the end of each mex call,
allowing the object to be used across multiple calls.

Couple this the mexInterface to create a class interface

This is intended to be used with +extras\+SessionManager\Session.m

/*--------------------------------------------------
Copyright 2018, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/

#pragma once

#include <unordered_map>
#include <memory>
#include <mex.h>
#include <list>

namespace extras{namespace SessionManager{


	template <class Obj>
	class ObjectManager{//: virtual public ObjectManager_Base {
	protected:
		std::unordered_map<intptr_t, std::shared_ptr<Obj>> ObjectMap;

		static intptr_t getIntPointer(const mxArray* pointer) {
			if (mxIsEmpty(pointer))
				throw(extras::stacktrace_error("ObjectManager:invalidType -> Id is empty."));
			if (sizeof(intptr_t) == 8 && !mxIsInt64(pointer) && !mxIsUint64(pointer))
				throw(extras::stacktrace_error("ObjectManager:invalidType -> Invalid ID type, pointer ID must be INT64 or UINT64."));
			if (sizeof(intptr_t) == 4 && !mxIsInt32(pointer) && !mxIsUint32(pointer))
				throw(extras::stacktrace_error("ObjectManager:invalidType -> Invalid ID type, pointer ID must be INT32 or UINT32."));
			return *reinterpret_cast<intptr_t*>(mxGetData(pointer));
		}

	public:

		/// destruct all managed objects
		void clearObjects() {
#ifdef _DEBUG
			mexPrintf("ObjectManager<%s>::clearObjects()\n", typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif
			// unlock mex file
			for (size_t n = 0; n < ObjectMap.size(); n++) {
				mexUnlock(); //decrement mex lock counter;
			}

			ObjectMap.clear(); //destroy all objects

#ifdef _DEBUG
			mexPrintf("\t...Objects cleared.\n", typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif
		}

		ObjectManager(){
#ifdef _DEBUG
			mexPrintf("Creating ObjectManager<%s>\n", typeid(Obj).name());
#endif
		}

		virtual ~ObjectManager(){
#ifdef _DEBUG
			mexPrintf("Destroying ObjectManager<%s>\n",typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif			
			clearObjects();
		}

		///Add object to map, creates a shared_ptr from the pointer
		/// call using something like objman.create(new YourObj());
		intptr_t create(Obj* p) { 
#ifdef _DEBUG
			mexPrintf("ObjectManager<%s>::create\n", typeid(Obj).name());
			mexEvalString("pause(0.2)");
#endif
			std::shared_ptr<Obj> newObj(p);
			intptr_t ptr = reinterpret_cast<intptr_t>(p);
			ObjectMap.insert(std::make_pair(ptr, newObj));//add object to map
			mexLock(); //increment mex lock counter;
			
			return ptr;
		}

		/// destroy instance
		void destroy(intptr_t id) {
#ifdef _DEBUG
			mexPrintf("ObjectManager<%s>::destroy(%d)\n", typeid(Obj).name(),id);
			mexEvalString("pause(0.2)");
#endif
			if (!ObjectMap.empty())
			{
				ObjectMap.erase(id);
				mexUnlock(); //deccrement mex lock counter;
			}
		}

		/// destroy instance specified in mxArray
		void destroy(const mxArray* in) {
			intptr_t id = getIntPointer(in);
			destroy(id);
		}

		std::shared_ptr<Obj> get(intptr_t id) {
#ifdef DAN_DEBUG
			mexPrintf("ObjectManager<%s>::get()\n\tnObjects=%d\n", typeid(Obj).name(),ObjectMap.size());
			mexEvalString("pause(0.2)");
#endif
			auto search = ObjectMap.find(id);
			if (search != ObjectMap.end()) {
				return search->second;
			}
			else {
				throw(extras::stacktrace_error(
					std::string("ObjectManager::get(")+
					std::to_string(id)+
					std::string(") -> Object not found")));
			}
		}

		std::shared_ptr<Obj> get(const mxArray* in) {
			intptr_t id = getIntPointer(in);
#ifdef DAN_DEBUG
			mexPrintf("\t ptr=%d\n", typeid(Obj).name(),id);
			mexEvalString("pause(0.2)");
#endif
			return get(id);
		}
	};
}}
