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

namespace extras {namespace cmex {

	// Struct-like wrapper around persistentMxArray which maps
	// a persistentMxArray with a string
	// Name,value pairs are set from a group of mxArray*
	// (e.g. arguments passed from at mexFunction, etc)
	//
	// The class should be thread-safe
	class ParameterMxMap {
		mutable std::mutex _mapMutex;
		std::unordered_map<std::string, extras::cmex::persistentMxArray> _map;

	public:
		// destructor
		~ParameterMxMap() {
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map
			_map.clear();
		}

		///////////////////////////
		// constructors

		// default constructor
		ParameterMxMap() {};

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

		// set map entry
		void setParameters(size_t nrhs, const mxArray* prhs[]) {
			if (nrhs % 2 != 0) {
				throw(std::runtime_error("ParameterMxMap::setParameters() number of args must be even (specified as Name,Value pairs)."));
			}

			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map

														 // loop over args and set parameters
			for (size_t n = 0; n < nrhs - 1; n += 2) {
				_map[extras::cmex::getstring(prhs[n])] = extras::cmex::persistentMxArray(prhs[n + 1]);
			}

		}

		// return parameters as (non-persistent) MxCellArray
		// NOTE: This will create copies of the data in the map, hence it can be slow for large arrays
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

		// return parameters as mxarraygroup
		// NOTE: This will create copies of the data in the map, hence it can be slow for large arrays
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

		// return number of entries in the map
		size_t size() const { return _map.size(); }

		// return reference to parameter field
		extras::cmex::persistentMxArray& operator[](const std::string& field) {
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map

			return _map[field];
		}

		// return const reference to parameter field
		const extras::cmex::persistentMxArray& operator[](const std::string& field) const {
			std::lock_guard<std::mutex> lock(_mapMutex); //lock map, prevent deleting until everyone is done messing with map

			return _map.at(field);
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
