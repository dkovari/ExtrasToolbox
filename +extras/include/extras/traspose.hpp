#pragma once
#include <cstddef>

namespace extras{
    const size_t REC_SZ = 16; //default recursive size for cachetran
    //internal use transpose
	template <typename T>
	void cachetran(size_t r_start, size_t r_end, size_t c_start, size_t c_end, const T* src, T* dst, size_t src_rows, size_t src_cols) {
		size_t delr = r_end - r_start;
		size_t delc = c_end - c_start;
		if (delr <= REC_SZ && delc <= REC_SZ) {
			for (size_t r = r_start; r < r_end; ++r) {
				for (size_t c = c_start; c < c_end; ++c) {
					dst[c*src_rows + r] = src[r*src_cols + c];
				}
			}
		}
		else if (delr >= delc) {
			cachetran(r_start, r_start + delr / 2, c_start, c_end, src, dst, src_rows, src_cols);
			cachetran(r_start + delr / 2, r_end, c_start, c_end, src, dst, src_rows, src_cols);
		}
		else {
			cachetran(r_start, r_end, c_start, c_start + delc / 2, src, dst, src_rows, src_cols);
			cachetran(r_start, r_end, c_start + delc / 2, c_end, src, dst, src_rows, src_cols);
		}
	}

	//transpose array
	template <typename T>
	void transpose_copy(const T* in, T* out, size_t in_m, size_t in_n) {
		cachetran(0, in_m, 0, in_n, in, out, in_m, in_n);
	}
}
