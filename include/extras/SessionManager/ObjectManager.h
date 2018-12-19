/*
Mex framework for creating C++ object which live past the end of each mex call,
allowing the object to be used across multiple calls.

Couple this the mexDispatch to create a class interface

This is intended to be used with Session.m
*/

#pragma once

#include <map>
#include <memory>
#include <mex.h>

namespace extras{namespace SessionManager{
	template <class Obj>
	class ObjectManager {
	protected:
		std::map<intptr_t, std::shared_ptr<Obj>> ObjectMap;

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
		ObjectManager() = default; //default constructor
		~ObjectManager(){
			mexPrintf("Destroying ObjectManager<%s>\n",typeid(Obj).name());
			size_t nObj = ObjectMap.size(); //number of objects created
			ObjectMap.clear(); //erase all objects
			for(size_t n=0;n<nObj;++n){
				mexUnlock(); //decrement mex lock counter.
			}
		}

		//Add object to map, creates a shared_ptr from the pointer
		// call using something like objman.create(new YourObj());
		intptr_t create(Obj* p) {
			//add object to map
			std::shared_ptr<Obj> newObj(p);
			intptr_t ptr = reinterpret_cast<intptr_t>(p);
			ObjectMap.insert(std::make_pair(ptr, newObj));

			mexLock(); //increment mex lock counter;

			return ptr;
		}

		//destroy instance
		void destroy(intptr_t id) {
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
			return get(id);
		}
	};
}}
