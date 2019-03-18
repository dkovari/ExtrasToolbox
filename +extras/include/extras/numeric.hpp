/*--------------------------------------------------
Copyright 2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#pragma once


namespace extras {
	/// return the product of all elements in a vector
	template <typename T> T prod(const std::vector<T>& vec) {
		if (vec.empty()) {
			return 0;
		}
		T out = 1;
		for (size_t n = 0; n<vec.size(); ++n) {
			out *= vec[n];
		}
		return out;
	}
}
