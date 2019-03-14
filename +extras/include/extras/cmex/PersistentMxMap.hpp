/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once


#include <extras/cmex/PersistentMxArray.hpp>
#include <unordered_map>
#include <mutex>
#include <extras/cmex/mexextras.hpp>
#include <extras/cmex/MxCellArray.hpp>
#include <extras/cmex/mxArrayGroup.hpp>
#include <memory>
#include <extras/string_extras.hpp>
#include <extras/cmex/MxStruct.hpp>

namespace extras {namespace cmex {

	// Struct-like wrapper around persistentMxArray which maps
	// a persistentMxArray with a string
	// Name,value pairs are set from a group of mxArray*
	// (e.g. arguments passed from at mexFunction, etc)
	//
	// The class should be thread-safe
	class ParameterMxMap {
	private:
		mutable std::mutex _mapMutex;
		std::unordered_map<std::string, extras::cmex::persistentMxArray> _map;
		bool _casesensitive = true;
		bool _setfromstruct = true;
	public:

		/////////////////
		// const info methods

		//! true if field names are case sensitive
		bool isCaseSensitive() const { return _casesensitive; }

		//! true if field names and values can be set by passing a struct array to setParameters()
		bool canSetFromStruct() const { return _setfromstruct; }

		// destructor
		virtual ~ParameterMxMap() {
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map
			_map.clear();
		}

		///////////////////////////
		// constructors

		// default constructor
		ParameterMxMap() {};

		//! create parameter map, specifying case sensitivity
		//! if casesensitive==true (default) all parameter names are converted to lowercase before being stored/retrieved from the map
		ParameterMxMap(bool casesensitive) : _casesensitive(casesensitive) {};

		//! create parameter map, specifying case sensitivity and struct-set ability
		//! if casesensitive all parameter names are converted to lowercase before being stored/retrieved from the map
		//! if setfromstruct==true (default), then setParameters() accepts a single mxArray* pointing to a struct-array
		ParameterMxMap(bool casesensitive, bool setfromstruct) : _casesensitive(casesensitive), _setfromstruct(setfromstruct) {};

		////////////////////
		// copy/move

		ParameterMxMap(const ParameterMxMap& src) {
			std::lock_guard<std::mutex> lock_src(src._mapMutex); //lock source map, prevent deleting until everyone is done messing with map
			_map = src._map;
		}
		ParameterMxMap& operator=(const ParameterMxMap& src) {
			std::lock_guard<std::mutex> lock_src(src._mapMutex); //lock source map, prevent deleting until everyone is done messing with map
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map
			_map = src._map;
			return *this;
		}

		ParameterMxMap(ParameterMxMap&& src) {
			std::lock_guard<std::mutex> lock_src(src._mapMutex); //lock source map, prevent deleting until everyone is done messing with map
			_map = std::move(src._map);
		}
		ParameterMxMap& operator=(ParameterMxMap&& src) {
			std::lock_guard<std::mutex> lock_src(src._mapMutex); //lock source map, prevent deleting until everyone is done messing with map
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map
			_map = std::move(src._map);
			return *this;
		}

		////////////////////////////////////
		// 

		//! set map entry
		//! calls operator[] to set each parameter, therefore if you redefine operator[] in
		//! a derived class you don't necessarily need to redefine this method
		virtual void setParameters(size_t nrhs, const mxArray* prhs[]) {

			mexPrintf("Inside ParameterMxMap::setParameters()\n");
			mexEvalString("pause(0.2)");

			if (_setfromstruct && nrhs==1) { // set from struct allowed
				if (!mxIsStruct(prhs[0])) {
					throw(std::runtime_error("ParameterMxMap::setParameters(): a single mxArray* was passed, but is was not a struct."));
				}

				if (!mxGetNumberOfElements(prhs[0]) != 1) {
					throw(std::runtime_error("ParameterMxMap::setParameters(): struct array must be a scalar struct (i.e. numel==1)"));
				}
				
				MxStruct thisStruct(prhs[0]);
				auto fnames = thisStruct.fieldnames();
				for (size_t n = 0; n < thisStruct.number_of_fields(); n++) {
					(*this)[fnames[n]] = thisStruct(0, fnames[n].c_str());
				}
			}

			if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParameterMxMap::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}

			 // loop over args and set parameters
			for (size_t n = 0; n < nrhs - 1; n += 2) {
				(*this)[extras::cmex::getstring(prhs[n])] = extras::cmex::persistentMxArray(prhs[n + 1]);
			}

		}

		//! return parameters as (non-persistent) MxCellArray
		//! NOTE: This will create copies of the data in the map, hence it can be slow for large arrays
		//! this method locks the _mapMutex while executing
		extras::cmex::MxCellArray map2cell() const {
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map

			extras::cmex::MxCellArray out;
			out.reshape(1, 2 * _map.size());

			size_t k = 0;
			for (const auto& p : _map) {
				out(k) = extras::cmex::MxObject(p.first);
				out(k + 1) = p.second.getMxArray();
				k += 2;
			}
			return out;
		}

		//! return parameters as mxarraygroup
		//! NOTE: This will create copies of the data in the map, hence it can be slow for large arrays
		//! this method locks the _mapMutex while executing
		extras::cmex::mxArrayGroup map2arraygroup() const {
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map

			extras::cmex::mxArrayGroup out(2 * _map.size());

			size_t k = 0;
			for (const auto& p : _map) {
				out.setArray(k, extras::cmex::MxObject(p.first));
				out.setArray(k + 1, p.second.getMxArray());
				k += 2;
			}
			return out;
		}

		//! return number of entries in the map
		size_t size() const { return _map.size(); }

		//! return reference to parameter field
		//! can be overloaded by derived class
		//! overriding derived methods should still call:
		//!		persistentMxArray::operator[](...) = ...
		//!	since the original version handles the internal map and mutex
		virtual extras::cmex::persistentMxArray& operator[](const std::string& field) {
			//std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map
			if (_casesensitive) {
				return _map[extras::tolower(field)];
			}
			return _map[field];
		}

		//! return const reference to parameter field
		//! can be overloaded by derived class
		virtual const extras::cmex::persistentMxArray& operator[](const std::string& field) const {
			//std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map
			if (_casesensitive) {
				return _map.at(extras::tolower(field));
			}
			return _map.at(field);
		}

		//! return true if string is existing parameter name
		//! does not lock interal mutex
		bool isparameter(const std::string& field) const {
			return _map.find(field) != _map.end();
		}

		//////////////////
		// Iterators

		typedef std::unordered_map<std::string, extras::cmex::persistentMxArray>::iterator iterator;
		typedef std::unordered_map<std::string, extras::cmex::persistentMxArray>::const_iterator const_iterator;
		iterator begin() { return _map.begin(); }
		iterator end() { return _map.end(); }
		const_iterator begin() const { return _map.begin(); }
		const_iterator end() const { return _map.end(); }

	};
}}
