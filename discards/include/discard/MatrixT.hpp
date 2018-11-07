#pragma once

// NOTE: Include mex.h before this header to enable mexMatrixT functionality MatrixT(mxArray*) construction
//#include <mex.h>

#include <stdexcept>
#include <algorithm>

#ifdef mex_h
	#include "mxClassIDhelpers.hpp"
#endif //end ifenf mex_h

#ifdef mex_h
template <typename M> class mexMatrixT;

// Copy data from c array with a given mxClassID type
template<typename T> void copyData(T* dst, void* src, mxClassID cID, size_t numel) {
	if (sametype(typeid(T), cID)) { //same type => simple copy
		memcpy(dst, src, numel * sizeof(T));
	}
	else { //different type, cast each element
		switch (cID) {
		case mxDOUBLE_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((double*)src)[n];
			}
			break;
		case mxSINGLE_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((float*)src)[n];
			}
			break;
		case mxINT8_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((int8_t*)src)[n];
			}
			break;
		case mxUINT8_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((uint8_t*)src)[n];
			}
			break;
		case mxINT16_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((int16_t*)src)[n];
			}
			break;
		case mxUINT16_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((uint16_t*)src)[n];
			}
			break;
		case mxINT32_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((int32_t*)src)[n];
			}
			break;
		case mxUINT32_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((uint32_t*)src)[n];
			}
			break;
		case mxINT64_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((int64_t*)src)[n];
			}
			break;
		case mxUINT64_CLASS:
			for (size_t n = 0; n < numel; ++n) {
				dst[n] = ((uint64_t*)src)[n];
			}
			break;
		default:
			throw(std::runtime_error("copyData: cannot copy from non-numeric ClassID."));
		}
	}
}

#endif //end ifenf mex_h

// MatrixT
// C++ (not MATLAB MEX) matrix class
template <typename T>
class MatrixT {
#ifdef mex_h
	template<typename M> friend class mexMatrixT;
#endif //end ifdef mex_h
protected:
	T* _data;
	size_t _M;
	size_t _N;
	virtual void freedata() {
		if (_data) {
			free(_data);
		}
		_M = 0;
		_N = 0;
		_data = nullptr;
	}
	virtual void mallocdata(size_t m, size_t n) {
		freedata();
		_data = (T*)malloc(m*n * sizeof(T));
		_M = m;
		_N = n;
	}
	virtual void mallocdata() {
		mallocdata(_M, _N);
	}
	virtual void callocdata(size_t m, size_t n) {
		freedata();
		_data = (T*)calloc(m*n, sizeof(T));
		_M = m;
		_N = n;
	}
	virtual void callocdata() {
		callocdata(_M, _N);
	}
	virtual void reallocdata(size_t m, size_t n) {
		if (_M*_N == m*n) {
			_M = m;
			_N = n;
			return;
		}
		_data = (T*)realloc(_data, m*n*sizeof(T));
		_M = m;
		_N = n;
	}
public:
	inline bool IsEmpty() const { return _M == 0 || _N == 0; }
	inline size_t datasize() { return _M*_N * sizeof(T); }
	inline size_t numel() const { return _M*_N; }
	inline size_t elementsize() const { return sizeof(T); }
	inline size_t nRows() const { return _M; }
	inline size_t nCols() const { return _N; }
	inline T* getdata() { return _data; }

	//destructor. delete data if needed
	~MatrixT() {
		freedata();
	}

	//default ctor, everything empty
	MatrixT() : _data(nullptr), _M(0), _N(0) {};

	//construct with size
	MatrixT(size_t M, size_t N, bool ZERO_DATA = false) : _data(nullptr), _M(M), _N(N) {
		if (ZERO_DATA) {
			callocdata();
		}
		else {
			mallocdata();
		}
	}

	//construct from c pointer
	MatrixT(size_t M, size_t N, const T* data):_data(nullptr),_M(M),_N(N){
		mallocdata();
		memcpy((void*)_data, (void*)data, datasize());
	}

	//copy constructor
	MatrixT(const MatrixT& other) : _data(nullptr), _M(other._M), _N(other._N){
		mallocdata();
		memcpy((void*)_data, (void*)other._data, datasize());
	}
	MatrixT& operator=(const MatrixT& other) {
		mallocdata(other._M, other._N);
		memcpy(_data, other._data, datasize());
		return *this;
	}

	//copy from different type
	template<typename M> MatrixT(const MatrixT<M>& other):_data(nullptr),_M(0),_N(0) {
		mallocdata(other._M, other._N);
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = (T)(other[i]); //copy by value with type conversion
		}
	}

	//move constructor
	MatrixT(MatrixT&& other) : _data(other._data),_M(other._M), _N(other._N){
		//mexPrintf("MatrixT(&&): %d\n",this);
		//mexPrintf("from move\n");
		other._data = nullptr;
		other._M = 0;
		other._N = 0;
	}
	MatrixT& operator=(MatrixT&& other) {
		freedata();
		_M = other._M;
		_N = other._N;
		_data = other._data;

		other._data = nullptr;
		other._M = 0;
		other._N = 0;

		return *this;
	}

	//Copy from same type
	MatrixT& copy(const MatrixT<T>& in) {
		mallocdata(in._M, in._N);
		memcpy(_data, in._data, datasize());
		return *this;
	}

	//copy from differnt type
	template<typename M>
	MatrixT& copy(const MatrixT<M>& in) {
		mallocdata(in._M, in._N);
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = T(in[i]); //copy with type conversion
		}
		return *this;
	}

	//resize
	virtual void resize(size_t M, size_t N) {
		if (_M*_N == M*N) {//same number of elements, just re-order
			_M = M;
			_N = N;
			return ;//*this;
		}

		if (M == 0 || N == 0) {
			M = 0;
			N = 0;
		}
		reallocdata(M,N);
		return ;//*this;
	}

	//resize without copying data (new data will be created with malloc and therefore not set to a particular value
	virtual void resize_nocpy(size_t M, size_t N) {
		if (_M*_N == M*N) {//same number of elements, just re-order
			_M = M;
			_N = N;
			return ;//*this;
		}

		if (M == 0 || N == 0) {
			M = 0;
			N = 0;
		}
		mallocdata(M, N);
		//return *this;
	}

	//set all to zero
	virtual void setzeros(){
		callocdata();
	}

	//resize set to zero
	virtual void resize_setzeros(size_t M, size_t N){
		callocdata(M,N);
	}

	//element access
	T& operator[](size_t i) {
		if (i >= numel()) { throw(std::runtime_error("MatrixT::[] index too large")); }
		return _data[i];
	}
	const T& operator[](size_t i) const {
		if (i >= numel()) { throw(std::runtime_error("MatrixT::[] index too large")); }
		return _data[i];
	}

	T& operator()(size_t i) {
		return (*this)[i];
	}
	const T& operator()(size_t i) const {
		return (*this)[i];
	}

	T& operator()(size_t m, size_t n) {
		return _data[m + n*nRows()];
	}
	const T& operator()(size_t m, size_t n) const {
		return _data[m + n*nRows()];
	}

	template<typename M> operator MatrixT<M>() {
		MatrixT<M> out(*this);
		return out;
	}


	//arithmetic
	MatrixT<double> operator-() {
		MatrixT<double> out(*this);
		for (size_t n = 0; n < numel(); ++n) {
			out._data[n] = -out._data[n];
		}
		return out;
	}

	MatrixT& operator++()//prefix operator
	{
		for (size_t i = 0; i < numel(); ++i) {
			++_data[i];
		}
		return *this;
	}
	MatrixT operator++(int) //suffix
	{
		MatrixT out(*this);
		for (size_t i = 0; i < numel(); ++i) {
			++_data[i];
		}
		return out;
	}

	MatrixT& operator--()//prefix operator
	{
		for (size_t i = 0; i < numel(); ++i) {
			--_data[i];
		}
		return *this;
	}
	MatrixT operator--(int) //suffix
	{
		MatrixT out(*this);
		for (size_t i = 0; i < numel(); ++i) {
			--_data[i];
		}
		return out;
	}

	template<typename M> MatrixT& operator+=(const MatrixT<M>& b) {
		if (numel() != b.numel()) throw(std::runtime_error("MatrixT+=MatrixT<M>: matricies must have same number of elements"));
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] += (T)(b._data[i]);
		}
		return *this;
	}
	template<typename M> MatrixT& operator-=(const MatrixT<M>& b) {
		if (numel() != b.numel()) throw(std::runtime_error("MatrixT-=MatrixT<M>: matricies must have same number of elements"));
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] -= (T)(b._data[i]);
		}
		return *this;
	}
	template<typename M> MatrixT& operator*=(const MatrixT<M>&) = delete; //not defined
	template<typename M> MatrixT& operator/=(const MatrixT<M>&) = delete; //not defined
	template<typename M> MatrixT operator*(const MatrixT<M>&) = delete; //not defined
	template<typename M> MatrixT operator/(const MatrixT<M>&) = delete; //not defined

	MatrixT& operator+=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] += b;
		}
		return *this;
	}
	MatrixT& operator-=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] -= b;
		}
		return *this;
	}
	MatrixT& operator*=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] *= b;
		}
		return *this;
	}
	MatrixT& operator/=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] /= b;
		}
		return *this;
	}

	MatrixT<T> operator+(const MatrixT<T>& b) //same type add
	{
		if (numel() != b.numel()) throw(std::runtime_error("MatrixT<T>+MatrixT<T>: matricies must have same number of elements"));
		MatrixT<T> out(*this);
		out += b;
		return out;
	}
	MatrixT<T> operator-(const MatrixT<T>& b) //same type subtract
	{
		if (numel() != b.numel()) throw(std::runtime_error("MatrixT<T>-MatrixT<T>: matricies must have same number of elements"));
		MatrixT<T> out(*this);
		out -= b;
		return out;
	}
	MatrixT<T> operator*(const T& v) {
		MatrixT<T> out(*this);
		out *= v;
		return out;
	}

	template<typename M> MatrixT<double> operator+(const MatrixT<M>& b) //different type add
	{
		if (numel() != b.numel()) throw(std::runtime_error("MatrixT<T>+MatrixT<M>: matricies must have same number of elements"));
		MatrixT<double> out(*this);
		out += b;
		return out;
	}
	template<typename M> MatrixT<double> operator-(const MatrixT<M>& b) //different type subtract
	{
		if (numel() != b.numel()) throw(std::runtime_error("MatrixT<T>-MatrixT<M>: matricies must have same number of elements"));
		MatrixT<double> out(*this);
		out -= b;
		return out;
	}

	MatrixT<T> operator+(const T& b) //same type
	{
		MatrixT<T> out(*this);
		out += b;
		return out;
	}
	template<typename M> MatrixT<double> operator+(const M& b) //dif type
	{
		MatrixT<double> out(*this);
		out += b;
		return out;
	}

	//dont use same type for -, might loose sign

	//dif type -
	template<typename M> MatrixT<double> operator-(const M& b) //dif type
	{
		MatrixT<double> out(*this);
		out -= b;
		return out;
	}
	
	//dif type *
	template<typename M> MatrixT<double> operator*(const M& b) {
		MatrixT<double> out(*this);
		out *= b;
		return out;
	}
	
	//dif type /
	template<typename M> MatrixT<double> operator/(const M& b) {
		MatrixT<double> out(*this);
		out /= b;
		return out;
	}

	template<typename V, typename M>
	friend MatrixT<double> operator-(const V&, const MatrixT<M>&);
	template<typename V, typename M>
	friend MatrixT<double> operator*(const V&, const MatrixT<M>&);

#ifdef mex_h
	//get data from mxArray*
	virtual MatrixT& copy(const mxArray* in) {
		mallocdata(mxGetM(in), mxGetN(in));
		copyData(_data, mxGetData(in), mxGetClassID(in), _M*_N);
		return *this;
	}

	//construct from mxArray*, copies cause this matrix in on c++ heap
	MatrixT(const mxArray* in) :_data(nullptr), _M(0), _N(0)
	{
		if (mxIsEmpty(in)) { return; } //was empty, return
		this->copy(in);
	}
	virtual MatrixT& operator=(const mxArray* in) {
		this->copy(in);
		return *this;
	}

	template<typename M> MatrixT(const mexMatrixT<M>& in) :_data(nullptr), _M(0), _N(0) {
		this->copy(in);
	}
	template<typename M> MatrixT& operator=(const mexMatrixT<M>& in) {
		this->copy(in);
		return *this;
	}

	void disp() const {
		mexPrintf("%d x %d %s=\n", _M, _N, classname(type2ClassID(typeid(T))));
		if (IsEmpty()) {
			mexPrintf("\t[ ]\n");
			return;
		}
		if (isint(type2ClassID(typeid(T)))) {
			for (size_t m = 0; m < _M; ++m) {
				for (size_t n = 0; n < _N; ++n) {
					mexPrintf("\t%d", int((*this)(m, n)));
				}
				mexPrintf("\n");
			}
		}
		else {
			for (size_t m = 0; m < _M; ++m) {
				for (size_t n = 0; n < _N; ++n) {
					mexPrintf("\t%f", double((*this)(m, n)));
				}
				mexPrintf("\n");
			}
		}
	}
#endif //end ifdef mex_h
};

template<typename T> MatrixT<T> operator+(const T& b, const MatrixT<T>& M) {
	MatrixT<T> out(M);
	out += b;
	return out;
}
template<typename T, typename M> MatrixT<double> operator+(const M& b, const MatrixT<T>& mat) {
	MatrixT<double> out(mat);
	out += b;
	return out;
}

template<typename V, typename M>
MatrixT<double> operator-(const V& v, const MatrixT<M>& m) {
	MatrixT<double> out(m.nRows(), m.nCols());
	for (size_t n = 0; n < m.numel(); ++n) {
		out._data[n] = v - m._data[n];
	}
	return out;
}

template<typename V, typename M>
MatrixT<double> operator*(const V& v, const MatrixT<M>& m) {
	MatrixT<double> out(m.nRows(), m.nCols());
	for (size_t n = 0; n < m.numel(); ++n) {
		out._data[n] = v * m._data[n];
	}
	return out;
}


#ifdef mex_h
template<typename M>
void disp(const MatrixT<M>& m) {
	m.disp();
}
#endif //end ifdef mex_h

template<typename M>
size_t numel(const MatrixT<M>& m) {
	return m.numel();
}
