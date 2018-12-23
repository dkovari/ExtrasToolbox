#pragma once

#include <string>
#include <cctype>
#include <string.h>

#ifndef strcmpi
const auto& strcmpi = strcasecmp;
#endif

// define tolower and toupper for strings ()
namespace extras{

    /// convert string to lowercase (in-place)
    std::string tolower(std::string && str) {
    	for (auto & c : str) c = std::tolower(c);
    	return str;
    }

    /// convert string to lowercase (make copy)
    std::string tolower(const std::string & str) {
    	std::string out(str);

    	for (auto & c : out) c = std::tolower(c);
    	return out;
    }

    /// convert string to upper case (in-place)
    std::string toupper(std::string && str) {
    	for (auto & c : str) c = std::toupper(c);
    	return str;
    }

    /// convert string to upper case (make copy)
    std::string toupper(const std::string &str) {
    	std::string out(str);
    	for (auto & c : out) c = std::toupper(c);
    	return out;
    }
}
