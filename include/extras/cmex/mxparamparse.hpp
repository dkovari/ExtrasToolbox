#pragma once

#include "mexextras.hpp"
#include "mxobject.hpp"
#include <unordered_map>
#include <string>
#include "../string_extras.hpp"

namespace extras{namespace cmex{

	///MEX argument parser, similar to MATLAB's inputParser
	//
    class MxInputParser {
    private:
        bool _CaseSensitive;
    public:
        std::unordered_map<std::string,MxObject> Parameter;
		std::unordered_map<std::string, bool>	ParameterFound;

        /// Create Parser with Case-sensitivity turned on
        MxInputParser():_CaseSensitive(true){};
        MxInputParser(bool CaseSensitive):_CaseSensitive(CaseSensitive){};

        ///Add Parameter, empty
        void AddParameter(std::string key){
            if(!_CaseSensitive) key = toupper(key);
            Parameter.emplace(key,MxObject());
			ParameterFound.emplace(key, false);
        }

        ///Add Parameter, Move contructor
        void AddParameter(std::string key, MxObject && MxObj){

            //mexPrintf("Move add\n");
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);
            Parameter.emplace(key,std::move(MxObj));
			ParameterFound.emplace(key, false);
        }
        ///Add Parameter, Copy Constructor
        void AddParameter(std::string key, const MxObject & MxObj){

            //mexPrintf("copy add\n");

            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);

            Parameter.emplace(key,MxObj);
			ParameterFound.emplace(key, false);
        }
        ///Add Parameter, Create value from const mxArray*
        void AddParameter(std::string key, const mxArray* mxptr){
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);

            Parameter.emplace(key,mxptr);
			ParameterFound.emplace(key, false);
        }

        ///AddParameter, Double scalar (also int with typecast)
        void AddParameter(std::string key, double val){
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);

            Parameter.emplace(key,val);
			ParameterFound.emplace(key, false);
        }
        void AddParameter(std::string key, int val){
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);

            Parameter.emplace(key,(double)val);
			ParameterFound.emplace(key, false);
        }

        ///AddParameter, string
        void AddParameter(std::string key, const std::string & val){
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);

            Parameter.emplace(key,val);
			ParameterFound.emplace(key, false);
        }

        /// Process Inputs
        int Parse(size_t nargs, const mxArray* pargs[]){

			if (nargs < 1) { return 0; }

            if(nargs%2 !=0){ //check that even number of inputs
                mexPrintf("Incorrect number of arguments passed to MxInputParser::Parse\n");
                return -1;
            }

            for(size_t n=0;n<nargs-1;n+=2){
                if(!mxIsChar(pargs[n])){
                    //not a string
                    mexPrintf("\n\targ[%d] is not a string\n",n);
                    mexPrintf("it is: %s\n",mxGetClassName(pargs[n]));
                    return 1;
                }
                //create key string
                std::string key = getstring(pargs[n]);

                //Convert to upper case if not case sensitive
                if(!_CaseSensitive) key = toupper(key);

                try{
                    Parameter.at(key) = pargs[n+1];
					ParameterFound.at(key) = true;
                }catch(std::out_of_range e){
                    //didn't find match
                    mexPrintf("arg[%d] could not find a match\n",n);
                    mexPrintf("\tusing key: '%s'\n",key.c_str());
                    mexPrintf("Possible Values:\n");
                    for(auto i:Parameter){
                        mexPrintf("\t'%s'\n",i.first.c_str());
                    }
                    return 2;
                }
            }
            return 0;
        }

        /// Return parameter, throws
        const mxArray* operator() (std::string key) const{
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);
            try{
                return Parameter.at(key).getmxarray();
            }catch(std::out_of_range e){

                mexPrintf("could not find match using key: '%s'\n",key.c_str());
                mexPrintf("Possible Values:\n");
                for(auto i:Parameter){
                    mexPrintf("'%s'\n",i.first.c_str());
                }

                return nullptr;
            }
        }
        const mxArray* operator() (const char* cstr) const{
            std::string key(cstr);
            //Convert to upper case if not case sensitive
            if(!_CaseSensitive) key = toupper(key);
            try{
                return Parameter.at(key).getmxarray();
            }catch(std::out_of_range e){

                mexPrintf("could not find match using key: '%s'\n",key.c_str());
                mexPrintf("Possible Values:\n");
                for(auto i:Parameter){
                    mexPrintf("'%s'\n",i.first.c_str());
                }

                return nullptr;
            }
        }

		bool wasFound(const char* cstr) const {
			std::string key(cstr);
			//Convert to upper case if not case sensitive
			if (!_CaseSensitive) key = toupper(key);
			try {
				return ParameterFound.at(key);
			}
			catch (std::out_of_range e) {

				mexPrintf("could not find match using key: '%s'\n", key.c_str());
				mexPrintf("Possible Values:\n");
				for (auto i : Parameter) {
					mexPrintf("'%s'\n", i.first.c_str());
				}

				return false;
			}
		}

    };
}}
