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

namespace extras{namespace SessionManager{
	template <class Obj>
	class ObjectManager {
	protected:
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

	public:
		ObjectManager(){
#ifdef DAN_DEBUG
			mexPrintf("Creating ObjectManager<%s>\n", typeid(Obj).name());
#endif
		}

		~ObjectManager(){
#ifdef DAN_DEBUG
			mexPrintf("Destroying ObjectManager<%s>\n",typeid(Obj).name());
#endif
			size_t nObj = ObjectMap.size(); //number of objects created
			ObjectMap.clear(); //erase all objects
			for(size_t n=0;n<nObj;++n){
				mexUnlock(); //decrement mex lock counter.
			}
		}

		//Add object to map, creates a shared_ptr from the pointer
		// call using something like objman.create(new YourObj());
		intptr_t create(Obj* p) { 
#ifdef DAN_DEBUG
			mexPrintf("ObjectManager<%s>::create\n", typeid(Obj).name());
#endif
			std::shared_ptr<Obj> newObj(p);
			intptr_t ptr = reinterpret_cast<intptr_t>(p);
			ObjectMap.insert(std::make_pair(ptr, newObj));//add object to map

			mexLock(); //increment mex lock counter;

			return ptr;
		}

		//destroy instance
		void destroy(intptr_t id) {
#ifdef DAN_DEBUG
			mexPrintf("ObjectManager<%s>::destroy(intptr_t id)\n", typeid(Obj).name());
#endif
			if (!ObjectMap.empty())
			{
				ObjectMap.erase(id);

				mexUnlock(); //deccrement mex lock counter;
			}
		}
		void destroy(const mxArray* in) {
			intptr_t id = getIntPointer(in);
			destroy(id);
		}

		std::shared_ptr<Obj> get(intptr_t id) {
#ifdef DAN_DEBUG
			mexPrintf("ObjectManager<%s>::get()\n\tnObjects=%d\n", typeid(Obj).name(),ObjectMap.size());
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
#ifdef DAN_DEBUG
			mexPrintf("\t ptr=%d\n", typeid(Obj).name(),id);
#endif
			return get(id);
		}
	};
}}
