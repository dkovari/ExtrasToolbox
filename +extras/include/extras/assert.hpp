#pragma once
#include <stdexcept>
#include <string>

namespace extras {
	//throws exception if cond==false
	void assert_condition(bool cond, const std::exception& except) {
		if (!cond) {
			throw(except);
		}
	}
	void assert_condition(bool cond, const std::string what) {
		assert_condition(cond, std::runtime_error(what));
	}

}
