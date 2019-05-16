#pragma once

#include <mex.h>
#include "MatrixT.hpp"
#include "mxClassIDhelpers.hpp"
#include <stdexcept>
#include "mxobject.hpp"

// MatrixT class extended to use MATLAB mxArray memory management
// mexMatrixT is derived from both MatrixT and mexMxObject
//
// mex::MxObject (from mxobject.hpp) provides a mxArray* wrapper
// and manages conversion to persistent memory
//
// MatrixT<t> is provides a templated interface for interacting with the 
// associated as if it were a simple data array
template <typename T>
class mexMatrixT : public MatrixT<T>, public mex::MxObject {
protected:
	void freedata() {
		deletemxptr();
		_M = 0;
		_N = 0;
		_data = nullptr;
	}

	void mallocdata(size_t m, size_t n) {
		bool wasPersistent = _isPersistent;
		freedata();
		_mxptr = mxCreateNumericMatrix(m, n, type2ClassID(typeid(T)), mxREAL);
		_data = (T*)mxGetData(_mxptr);
		_managemxptr = true;
		_M = m;
		_N = n;
		if (wasPersistent) { makePersistent(); }
	}

	void callocdata(size_t m, size_t n) {
		bool wasPersistent = _isPersistent;
		freedata();
		_mxptr = mxCreateNumericMatrix(0, 0, type2ClassID(typeid(T)), mxREAL);
		mxSetM(_mxptr, m);
		mxSetN(_mxptr, n);
		_data = (T*)mxCalloc(m*n, sizeof(T));
		mxSetData(_mxptr, _data);
		_M = m;
		_N = n;
		if (wasPersistent) { makePersistent(); }
	}

	void reallocdata(size_t m, size_t n) {
		if (_M == m && _N == n) {
			return;
		}

		if (_managemxptr && _M*_N == m*n) { //no size change
			mxSetM(_mxptr, m);
			mxSetN(_mxptr, n);
			_M = m;
			_N = n;
			return;
		}

		if (m*n <= _M*_N) { //just copy upto range
			mxArray* newArray = mxCreateNumericMatrix(m, n, type2ClassID(typeid(T)), mxREAL);
			T* newdata = (T*)mxGetData(newArray);
			memcpy(newdata, mxGetData(_mxptr), m*n * sizeof(T));

			bool wasPersistent = _isPersistent;
			freedata();

			_data = newdata;
			_mxptr = newArray;
			_M = m;
			_N = n;
			_managemxptr = true;
			if (wasPersistent) { mexMakeArrayPersistent(_mxptr); }
		}
		else { // new size is bigger, set new data to zero
			mxArray* newArray = mxCreateNumericMatrix(0, 0, type2ClassID(typeid(T)), mxREAL);
			T* newdata = (T*)mxCalloc(m*n, sizeof(T));
			memcpy(newdata, mxGetData(_mxptr), m*n * sizeof(T));
			mxSetM(newArray, m);
			mxSetN(newArray, n);
			mxSetData(newArray, newdata);

			bool wasPersistent = _isPersistent;
			freedata();

			_data = newdata;
			_mxptr = newArray;
			_M = m;
			_N = n;
			_managemxptr = true;
			if (wasPersistent) { mexMakeArrayPersistent(_mxptr); }
		}
	}

public:

	size_t numel() const { return _M*_N; } //overload specifically since both mex::MxObject and MatrixT have numel

	//default ctor, everything empty
	mexMatrixT() { _data = nullptr; _M = 0; _N = 0; };

	//construct with size
	mexMatrixT(size_t M, size_t N, bool ZERO_DATA = false) : _data(nullptr), _M(M), _N(N) {
		if (ZERO_DATA) {
			callocdata();
		}
		else {
			mallocdata();
		}
	}

	//construct from c pointer
	mexMatrixT(size_t M, size_t N, const T* data) :_data(nullptr), _M(M), _N(N) {
		mallocdata();
		memcpy((void*)_data, (void*)data, datasize());
	}

	//copy constructor
	mexMatrixT(const MatrixT& other) : _data(nullptr), _M(other._M), _N(other._N) {
		mallocdata();
		memcpy((void*)_data, (void*)other._data, datasize());
	}
	mexMatrixT& operator=(const MatrixT& other) {
		mallocdata(other._M, other._N);
		memcpy(_data, other._data, datasize());
		return *this;
	}

	mexMatrixT(const mex::MxObject& other) {
		if (!other.isnumeric()) {
			throw(std::runtime_error("mexMatrixT: cannot construct from mex::MxObject with non-numeric data type"));
		}

		this->copy(other._mxptr);

	}
	mexMatrixT& operator=(const mex::MxObject& other) {
		if (!other.isnumeric()) {
			throw(std::runtime_error("mexMatrixT: cannot construct from mex::MxObject with non-numeric data type"));
		}
		freedata();
		this->copy(other._mxptr);
	}

	mexMatrixT(mex::MxObject&& other) {
		if (!other.isnumeric()) {
			throw(std::runtime_error("mexMatrixT: cannot construct from mex::MxObject with non-numeric data type"));
		}
		if (other._mxptr == nullptr) {
			_mxptr = nullptr;
			_managemxptr = true;
			_isPersistent = false;
			_M = 0;
			_N = 0;
			_data = nullptr;
		}
		else {

			if (!sametype(typeinfo(T), other._mxptr)) {
				this->copy(other._mxptr);
			}
			else{
				_mxptr = other._mxptr;
				_managemxptr = other._managemxptr;
				other._managemxptr = false;
				_isPersistent = other._isPersistent;

				_M = mxGetM(_mxptr);
				_N = mxGetN(_mxptr);
				_data = mxGetData(_mxptr);
			}
		}
	}

	mexMatrixT& operator=(mex::MxObject&& other) {
		if (!other.isnumeric()) {
			throw(std::runtime_error("mexMatrixT: cannot construct from mex::MxObject with non-numeric data type"));
		}
		freedata();

		if (other._mxptr == nullptr) {
			_mxptr = nullptr;
			_managemxptr = true;
			_isPersistent = false;
			_M = 0;
			_N = 0;
			_data = nullptr;
		}
		else {

			if (!sametype(typeinfo(T), other._mxptr)) {
				this->copy(other._mxptr);
			}
			else {
				_mxptr = other._mxptr;
				_managemxptr = other._managemxptr;
				other._managemxptr = false;
				_isPersistent = other._isPersistent;

				_M = mxGetM(_mxptr);
				_N = mxGetN(_mxptr);
				_data = mxGetData(_mxptr);
			}
		}
		return *this;
	}

	//copy from different type
	template<typename M> mexMatrixT(const MatrixT<M>& other){
		mallocdata(other._M, other._N);
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = (T)(other[i]); //copy by value with type conversion
		}
	}
	template<typename M> mexMatrixT& operator=(const MatrixT<M>& other) {
		mallocdata(other._M, other._N);
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = (T)(other[i]); //copy by value with type conversion
		}
		return *this;
	}

	//move ctor
	mexMatrixT(mexMatrixT&& other) {
		_mxptr = other._mxptr;
		_data = other._data;
		_M = other._M;
		_N = other._N;
		_managemxptr = other._managemxptr;
		_isPersistent = other._isPersistent;

		other._mxptr = nullptr;
		other._data = nullptr;
		other._M = 0;
		other._N = 0;
		other._managemxptr = true;
		other._isPersistent = false;
	}

	//move assign
	mexMatrixT& operator=(mexMatrixT&& other) {
		freedata();
		_mxptr = other._mxptr;
		_data = other._data;
		_M = other._M;
		_N = other._N;
		_managemxptr = other._managemxptr;
		_isPersistent = other._isPersistent;

		other._mxptr = nullptr;
		other._data = nullptr;
		other._M = 0;
		other._N = 0;
		other._managemxptr = true;
		other._isPersistent = false;

		return *this;
	}

	//construct from mutable mxArray*
	mexMatrixT(mxArray* in) {
		if (!sametype(typeid(T), in)) { //different type
			this->copy(in);
		}
		else { // same type
			_mxptr = in;
			_data = mxGetData(in);
			_M = mxGetM(in);
			_N = mxGetN(in);
			_managemxptr = false;
			_isPersistent = false;
		}
	}

	//assign from mutable mxArray*
	mexMatrixT& operator=(mxArray* in) {
		if (!sametype(typeid(T), in)) { //different type
			this->copy(in);
		}
		else { // same type
			freedata();
			_mxptr = in;
			_data = mxGetData(in);
			_M = mxGetM(in);
			_N = mxGetN(in);
			_managemxptr = false;
			_isPersistent = false;
		}

		return *this;
	}

	//construct from const mxArray*
	mexMatrixT(const mxArray* in) {
		this->copy(in);
	}

	//assign from const mxArray*
	mexMatrixT& operator=(const mxArray* in) {
		this->copy(in);
		return *this;
	}
};

template<typename T> mexMatrixT<T> operator+(const T& b, const mexMatrixT<T>& M) {
	mexMatrixT<T> out(M);
	out += b;
	return out;
}
template<typename T, typename M> mexMatrixT<double> operator+(const M& b, const mexMatrixT<T>& mat) {
	mexMatrixT<double> out(mat);
	out += b;
	return out;
}

template<typename V, typename M>
mexMatrixT<double> operator-(const V& v, const mexMatrixT<M>& m) {
	mexMatrixT<double> out(m.nRows(), m.nCols());
	for (size_t n = 0; n < m.numel(); ++n) {
		out._data[n] = v - m._data[n];
	}
	return out;
}

template<typename V, typename M>
mexMatrixT<double> operator*(const V& v, const mexMatrixT<M>& m) {
	mexMatrixT<double> out(m.nRows(), m.nCols());
	for (size_t n = 0; n < m.numel(); ++n) {
		out._data[n] = v * m._data[n];
	}
	return out;
}
