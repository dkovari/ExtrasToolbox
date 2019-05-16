/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once

#include <string>
#include <cctype>
#include <string.h>
#include <string>
#include <vector>

#ifndef _WIN32 //on mac use strcasecmp
#ifndef strcmpi
const auto& strcmpi = strcasecmp;
#endif
#else //on windows use _strcmpi
#ifndef strcmpi
#define strcmpi _strcmpi
#endif
#endif

// define tolower and toupper for strings ()
namespace extras{

    //! convert string to lowercase (in-place)
    std::string tolower(std::string && str) {
    	for (auto & c : str) c = std::tolower(c);
    	return str;
    }

    //! convert string to lowercase (make copy)
    std::string tolower(const std::string & str) {
    	std::string out(str);

    	for (auto & c : out) c = std::tolower(c);
    	return out;
    }

    //! convert string to upper case (in-place)
    std::string toupper(std::string && str) {
    	for (auto & c : str) c = std::toupper(c);
    	return str;
    }

    //! convert string to upper case (make copy)
    std::string toupper(const std::string &str) {
    	std::string out(str);
    	for (auto & c : out) c = std::toupper(c);
    	return out;
    }

	//! return true if string is found in a vector of strings
	//! optionally specify if search is case sensitive (default = true)
	bool ismember(const std::string& str, const std::vector<std::string>& str_list,bool CaseSensitive=true) {
		for (const auto& s2 : str_list) {
			if (CaseSensitive) {
				if (strcmp(str.c_str(), s2.c_str())==0) {
					return true;
				}
			}
			else {
				if (strcmpi(str.c_str(), s2.c_str()) == 0) {
					return true;
				}
			}
		}
		return false;
	}
}
