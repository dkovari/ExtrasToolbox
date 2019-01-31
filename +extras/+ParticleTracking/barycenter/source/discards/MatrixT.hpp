#pragma once

#include <mex.h>
#include <algorithm>
#include "mxClassIDhelpers.hpp"

template <typename M> class mexMatrixT;

// MatrixT
// C++ (not MATLAB MEX) matrix class
template <typename T>
class MatrixT {
	template<typename M> friend class mexMatrixT;
protected:
	T* _data;
	size_t _M;
	size_t _N;
	void freedata() {
		//mexPrintf("MatrixT::freedata()\n");
		if (_data) {
			//mexPrintf("Matrix: %d -> freeing _data-%d\n",this,_data);
			free(_data);
		}
		_M = 0;
		_N = 0;
		_data = nullptr;
	}
	void mallocdata(size_t m, size_t n) {
		//mexPrintf("MatrixT::mallocdata()\n");
		freedata();
		_data = (T*)malloc(m*n * sizeof(T));
		//mexPrintf("Matrix: %d -> malloc _data-%d\n",this,_data);
		_M = m;
		_N = n;
	}
	void mallocdata() {
		mallocdata(_M, _N);
	}
	void callocdata(size_t m, size_t n) {
		//mexPrintf("MatrixT::callocdata()\n");
		freedata();
		_data = (T*)calloc(m*n, sizeof(T));
		//mexPrintf("Matrix: %d -> calloc _data-%d\n",this,_data);
		_M = m;
		_N = n;
	}
	void callocdata() {
		callocdata(_M, _N);
	}

	void reallocdata(size_t m, size_t n) {
		//mexPrintf("MatrixT::reallocdata()\n");
		if (_M*_N == m*n) {
			_M = m;
			_N = n;
			return;
		}
		_data = (T*)realloc(_data, m*n*sizeof(T));
		//mexPrintf("Matrix: %d -> realloc _data-%d\n",this,_data);
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
		//mexPrintf("~MatrixT: %d _data=%d\n",this,_data);
	}
	//default ctor, everything empty
	MatrixT() : _data(nullptr), _M(0), _N(0) {
		//mexPrintf("MatrixT(): %d\n",this);
	};
	//construct with size
	MatrixT(size_t M, size_t N, bool ZERO_DATA = false) : _data(nullptr), _M(M), _N(N) {
		//mexPrintf("MatrixT(M,N,ZD): %d\n",this);
		//mexPrintf("size\n");
		if (ZERO_DATA) {
			callocdata();
		}
		else {
			mallocdata();
		}
	}

	MatrixT(size_t M, size_t N, T* data):_data(nullptr),_M(M),_N(N){
		//mexPrintf("from data\n");
		//mexPrintf("MatrixT(M,N,data*): %d\n",this);
		mallocdata();
		memcpy((void*)_data, (void*)data, datasize());
	}

	//copy constructor
	MatrixT(const MatrixT& other) : _data(nullptr), _M(other._M), _N(other._N){
		//mexPrintf("from copy\n");
		//mexPrintf("MatrixT(const &): %d\n",this);
		mallocdata();
		memcpy((void*)_data, (void*)other._data, datasize());
	}
	MatrixT& operator=(const MatrixT& other) {
		//mexPrintf("Copy assign MatrixT:%d\n",this);
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
		//mexPrintf("move assign MatrixT=&&: %d\n",this);
		//mexPrintf("move=\n");
		freedata();
		_M = other.M;
		_N = other._N;
		_data = other._data;

		other._data = nullptr;
		other._M = 0;
		other._N = 0;
	}

	//get data from mxArray*
	MatrixT& copy(const mxArray* in) {
		//mexPrintf("copy from mx MatrixT(const mx): %d\n",this);
		//mexPrintf("MatrixT::copy(mxArray*)\n");
		freedata();
		if (mxIsEmpty(in)) { return *this; } //empty
		_M = mxGetM(in);
		_N = mxGetN(in);
		mallocdata();

		if (!sametype(typeid(T), mxGetClassID(in))) {
			void* data_in = mxGetData(in);
			switch (mxGetClassID(in)) {
			case mxDOUBLE_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((double*)data_in)[n];
				}
				break;
			case mxSINGLE_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((float*)data_in)[n];
				}
				break;
			case mxINT8_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int8_t*)data_in)[n];
				}
				break;
			case mxUINT8_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint8_t*)data_in)[n];
				}
				break;
			case mxINT16_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int16_t*)data_in)[n];
				}
				break;
			case mxUINT16_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint16_t*)data_in)[n];
				}
				break;
			case mxINT32_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int32_t*)data_in)[n];
				}
				break;
			case mxUINT32_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint32_t*)data_in)[n];
				}
				break;
			case mxINT64_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int64_t*)data_in)[n];
				}
				break;
			case mxUINT64_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint64_t*)data_in)[n];
				}
				break;
			default:
				mexErrMsgTxt("MatrixT::copy(mxArray*): cannot copy from non-numeric array.");
			}
		}
		else {
			memcpy((void*)_data, mxGetData(in), datasize());
		}
		return *this;
	}
	//construct from mxArray*, copies cause this matrix in on c++ heap
	MatrixT(const mxArray* in) :_data(nullptr), _M(0),_N(0)
	{
		//mexPrintf("MatrixT(const mx):%d from mxArray\n",this);
		if (mxIsEmpty(in)) { return; } //was empty, return
		this->copy(in);
	}
	MatrixT& operator=(const mxArray* in) {
		//mexPrintf("assign from mxArray\n");
		this->copy(in);
		return *this;
	}

	//copy/contrusct from mexMatrixT
	MatrixT& copy(const mexMatrixT<T>& in) {
		freedata();
		mallocdata(in._M, in._N);
		memcpy(_data, in._data, datasize());
		return *this;
	}
	template<typename M> MatrixT& copy(const mexMatrixT<M>& in) {
		freedata();
		mallocdata(in._M, in._N);
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = T(in[i]); //copy with type conversion
		}
	}
	template<typename M> MatrixT(const mexMatrixT<M>& in):_data(nullptr),_M(0),_N(0) {
		this->copy(in);
	}
	template<typename M> MatrixT& operator=(const mexMatrixT<M>& in) {
		this->copy(in);
		return *this;
	}

	//resize
	void resize(size_t M, size_t N) {
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
	void resize_nocpy(size_t M, size_t N) {
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
	void setzeros(){
		callocdata();
	}

	//resize set to zero
	void resize_setzeros(size_t M, size_t N){
		callocdata(M,N);
	}

	//element access
	T& operator[](size_t i) {
		/*while (i < 0) { //negative index goes to end
			i += numel();
		}
		while (i >= numel()) { ////beyond end wraps to beginning
			i -= numel();
		}*/
		if (i >= numel()) { mexErrMsgTxt("MatrixT::[] index too large"); }
		return _data[i];
	}
	const T& operator[](size_t i) const {
		/*while (i < 0) { //negative index goes to end
			i = numel() + i;
		}
		while (i >= numel()) { ////beyond end wraps to beginning
			i = i - numel();
		}*/
		if (i >= numel()) { mexErrMsgTxt("MatrixT::[] index too large"); }
		return _data[i];
	}

	T& operator()(size_t i) {
		return (*this)[i];
	}
	const T& operator()(size_t i) const {
		return (*this)[i];
	}

	T& operator()(size_t m, size_t n) {
		/*while (m < 0) {
			m += nRows();
		}
		while (m >= nRows()) {
			m -= nRows();
		}
		while (n < 0) {
			n += nCols();
		}
		while (n >= nCols()) {
			n -= nCols();
		}*/
		//if (i >= numel()) { mexErrMsgTxt("MatrixT::[] index too large"); }
		return _data[m + n*nRows()];
	}
	const T& operator()(size_t m, size_t n) const {
		/*while (m < 0) {
			m += nRows();
		}
		while (m >= nRows()) {
			m -= nRows();
		}
		while (n < 0) {
			n += nCols();
		}
		while (n >= nCols()) {
			n -= nCols();
		}*/
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
		if (numel() != b.numel()) mexErrMsgTxt("MatrixT+=MatrixT<M>: matricies must have same number of elements");
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] += (T)(b._data[i]);
		}
		return *this;
	}
	template<typename M> MatrixT& operator-=(const MatrixT<M>& b) {
		if (numel() != b.numel()) mexErrMsgTxt("MatrixT-=MatrixT<M>: matricies must have same number of elements");
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] -= (T)(b._data[i]);
		}
		return *this;
	}
	template<typename M> MatrixT& operator*=(const MatrixT<M>&); //not defined
	template<typename M> MatrixT& operator/=(const MatrixT<M>&); //not defined
	template<typename M> MatrixT operator*(const MatrixT<M>&); //not defined
	template<typename M> MatrixT operator/(const MatrixT<M>&); //not defined

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
		if (numel() != b.numel()) mexErrMsgTxt("MatrixT<T>+MatrixT<T>: matricies must have same number of elements");
		MatrixT<T> out(*this);
		out += b;
		return out;
	}
	MatrixT<T> operator-(const MatrixT<T>& b) //same type subtract
	{
		if (numel() != b.numel()) mexErrMsgTxt("MatrixT<T>-MatrixT<T>: matricies must have same number of elements");
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
		if (numel() != b.numel()) mexErrMsgTxt("MatrixT<T>+MatrixT<M>: matricies must have same number of elements");
		MatrixT<double> out(*this);
		out += b;
		return out;
	}
	template<typename M> MatrixT<double> operator-(const MatrixT<M>& b) //different type subtract
	{
		if (numel() != b.numel()) mexErrMsgTxt("MatrixT<T>-MatrixT<M>: matricies must have same number of elements");
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


template<typename M>
void disp(const MatrixT<M>& m) {
	m.disp();
}

template<typename M>
size_t numel(const MatrixT<M>& m) {
	return m.numel();
}


template <typename T>
class mexMatrixT {
	template<typename M> friend class MatrixT;
protected:
	T* _data;
	size_t _M;
	size_t _N;
	mutable bool _mxArray;

	void freedata() {
		if (!_mxArray && _data) {
			mxFree(_data);
		}
		_M = 0;
		_N = 0;
		_data = nullptr;
		_mxArray = false;
	}
	void mallocdata(size_t m, size_t n) {
		freedata();
		_data = (T*)mxMalloc(m*n * sizeof(T));
		_M = m;
		_N = n;
	}
	void mallocdata() {
		mallocdata(_M, _N);
	}
	void callocdata(size_t m, size_t n) {
		freedata();
		_data = (T*)mxCalloc(m*n, sizeof(T));
		_M = m;
		_N = n;
	}
	void callocdata() {
		callocdata(_M, _N);
	}

	void reallocdata(size_t m, size_t n) {
		if (_M*_N == m*n) {
			_M = m;
			_N = n;
			return;
		}
		_data = (T*)mxRealloc(_data, m*n * sizeof(T));
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
	~mexMatrixT() {
		freedata();
	}
	//default ctor, everything empty
	mexMatrixT() : _data(nullptr), _M(0), _N(0), _mxArray(false) {};
	//construct with size
	mexMatrixT(size_t M, size_t N, bool ZERO_DATA = false) : _data(nullptr), _M(M), _N(N), _mxArray(false) {
		if (ZERO_DATA) {
			callocdata();
		}
		else {
			mallocdata();
		}
	}
	//construct by copying data
	mexMatrixT(size_t M, size_t N, T* data) : _data(nullptr), _M(M), _N(N), _mxArray(false) {
		mallocdata();
		memcpy((void*)_data, (void*)data, datasize());
	}

	//copy constructor
	mexMatrixT(const mexMatrixT& other) : _data(nullptr), _M(other._M), _N(other._N), _mxArray(false) {
		mallocdata();
		memcpy((void*)_data, (void*)other._data, datasize());
	}
	mexMatrixT& operator=(const mexMatrixT& other) {
		mallocdata(other._M, other._N);
		memcpy(_data, other._data, datasize());
		return *this;
	}
	//copy from different type
	template<typename M> mexMatrixT(const mexMatrixT<M>& other) :_data(nullptr), _M(0), _N(0), _mxArray(false) {
		mallocdata(other._M, other._N);

		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = (T)(other[i]); //copy by value with type conversion
		}
	}

	//move constructor
	mexMatrixT(mexMatrixT&& other) : _data(other._data), _M(other._M), _N(other._N), _mxArray(other._mxArray) {
		other._data = nullptr;
		other._M = 0;
		other._N = 0;
		other._mxArray = false;
	}
	mexMatrixT& operator=(mexMatrixT&& other) {
		freedata();
		_mxArray = other._mxArray;
		_M = other.M;
		_N = other._N;
		_data = other._data;

		other._data = nullptr;
		other._M = 0;
		other._N = 0;
		other._mxArray = false;
	}

	//get data from mxArray*
	mexMatrixT& copy(const mxArray* in) {
		freedata();
		if (mxIsEmpty(in)) { return *this; } //empty
		_M = mxGetM(in);
		_N = mxGetN(in);
		mallocdata();

		if (!sametype(typeid(T), mxGetClassID(in))) {
			void* data_in = mxGetData(in);
			switch (mxGetClassID(in)) {
			case mxDOUBLE_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((double*)data_in)[n];
				}
				break;
			case mxSINGLE_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((float*)data_in)[n];
				}
				break;
			case mxINT8_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int8_t*)data_in)[n];
				}
				break;
			case mxUINT8_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint8_t*)data_in)[n];
				}
				break;
			case mxINT16_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int16_t*)data_in)[n];
				}
				break;
			case mxUINT16_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint16_t*)data_in)[n];
				}
				break;
			case mxINT32_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int32_t*)data_in)[n];
				}
				break;
			case mxUINT32_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint32_t*)data_in)[n];
				}
				break;
			case mxINT64_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((int64_t*)data_in)[n];
				}
				break;
			case mxUINT64_CLASS:
				for (size_t n = 0; n < numel(); ++n) {
					_data[n] = ((uint64_t*)data_in)[n];
				}
				break;
			default:
				mexErrMsgTxt("mexMatrixT::copy(mxArray*): cannot copy from non-numeric array.");
			}
		}
		else {
			memcpy((void*)_data, mxGetData(in), datasize());
		}
		return *this;
	}
	//construct from mxArray*, does not copy
	mexMatrixT(const mxArray* in) :_data(nullptr), _M(0), _N(0), _mxArray(false)
	{
		if (mxIsEmpty(in)) { return; } //was empty, return
		if (!sametype(typeid(T), mxGetClassID(in))) {
			this->copy(in);
			return;
		}
		_M = mxGetM(in);
		_N = mxGetN(in);
		_mxArray = true;
		_data = (T*)mxGetData(in);
	}
	mexMatrixT& operator=(const mxArray* in) {
		freedata();
		if (!sametype(typeid(T), mxGetClassID(in))) {
			this->copy(in);
			return *this;
		}
		_M = mxGetM(in);
		_N = mxGetN(in);
		_mxArray = true;
		_data = (T*)mxGetData(in);
		return *this;
	}

	//pass data to mxArray*
	// note: If you resize the array after this opperation they link between the generated
	// mxArray pointer and the internal data is broken and a new data array is created.
	// Consequently, if you need to free the original array, you need to do that manually using mxFree(mxGetData(ptr=Cast MxArray Ptr))
	// Generally speaking, only cast to mxArrays at the end of a program when you need to return output.
	operator mxArray*() const {
		mxArray* out = mxCreateNumericMatrix(0, 0, type2ClassID(typeid(T)), mxREAL);
		mxSetData(out, (void*)_data);
		mxSetM(out, _M);
		mxSetN(out, _N);
		_mxArray = true;
		return out;
	}

	//dissacociate data from mxArray (copy self)
	mexMatrixT& ReleaseMxArray() {
		if (!_mxArray || IsEmpty()) {
			return *this;
		}
		T* tmp = _data;
		mallocdata();
		memcpy(_data, tmp, datasize());
		_mxArray = false;
		return *this;
	}

	//copy/contrusct from MatrixT
	mexMatrixT& copy(const MatrixT<T>& in) {
		freedata();
		mallocdata(in._M, in._N);
		memcpy(_data, in._data, datasize());
		return *this;
	}
	template<typename M> mexMatrixT& copy(const MatrixT<M>& in) {
		freedata();
		mallocdata(in._M, in._N);
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] = T(in[i]); //copy with type conversion
		}
	}
	template<typename M> mexMatrixT(const MatrixT<M>& in) :_data(nullptr), _M(0), _N(0),_mxArray(false) {
		this->copy(in);
	}
	template<typename M> mexMatrixT& operator=(const MatrixT<M>& in) {
		this->copy(in);
		return *this;
	}

	//resize
	void resize(size_t M, size_t N) {
		if (_M*_N == M*N) {//same number of elements, just re-order
			_M = M;
			_N = N;
			return;// *this;
		}

		if (M == 0 || N == 0) {
			M = 0;
			N = 0;
		}
		ReleaseMxArray();
		reallocdata(M, N);
		return;// *this;
	}

	//resize w/o copy
	//resize
	void resize_nocpy(size_t M, size_t N) {
		if (_M*_N == M*N) {//same number of elements, just re-order
			_M = M;
			_N = N;
			return ;//*this;
		}

		if (M == 0 || N == 0) {
			M = 0;
			N = 0;
		}
		_mxArray = false;
		mallocdata(M, N);
		//return *this;
	}

	//set all to zero
	void setzeros(){
		callocdata();
	}

	//resize set to zero
	void resize_setzeros(size_t M, size_t N){
		_mxArray = false;
		callocdata(M,N);
	}

	//element access
	T& operator[](size_t i) {
		/*while (i < 0) { //negative index goes to end
			i += numel();
		}
		while (i >= numel()) { ////beyond end wraps to beginning
			i -= numel();
		}*/
		if (i >= numel()) { mexErrMsgTxt("MatrixT::[] index too large"); }
		return _data[i];
	}
	const T& operator[](size_t i) const {
		/*while (i < 0) { //negative index goes to end
			i = numel() + i;
		}
		while (i >= numel()) { ////beyond end wraps to beginning
			i = i - numel();
		}*/
		if (i >= numel()) { mexErrMsgTxt("MatrixT::[] index too large"); }
		return _data[i];
	}

	T& operator()(size_t i) {
		return (*this)[i];
	}
	const T& operator()(size_t i) const {
		return (*this)[i];
	}

	T& operator()(size_t m, size_t n) {
		/*while (m < 0) {
			m += nRows();
		}
		while (m >= nRows()) {
			m -= nRows();
		}
		while (n < 0) {
			n += nCols();
		}
		while (n >= nCols()) {
			n -= nCols();
		}*/
		return _data[m + n*nRows()];
	}
	const T& operator()(size_t m, size_t n) const {
		/*while (m < 0) {
			m += nRows();
		}
		while (m >= nRows()) {
			m -= nRows();
		}
		while (n < 0) {
			n += nCols();
		}
		while (n >= nCols()) {
			n -= nCols();
		}*/
		return _data[m + n*nRows()];
	}

	template<typename M> operator mexMatrixT<M>() {
		mexMatrixT<M> out(*this);
		return out;
	}


	//arithmetic
	mexMatrixT<double> operator-() {
		mexMatrixT<double> out(*this);
		for (size_t n = 0; n < numel(); ++n) {
			out._data[n] = -out._data[n];
		}
		return out;
	}

	mexMatrixT& operator++()//prefix operator
	{
		for (size_t i = 0; i < numel(); ++i) {
			++_data[i];
		}
		return *this;
	}
	mexMatrixT operator++(int) //suffix
	{
		mexMatrixT out(*this);
		for (size_t i = 0; i < numel(); ++i) {
			++_data[i];
		}
		return out;
	}

	mexMatrixT& operator--()//prefix operator
	{
		for (size_t i = 0; i < numel(); ++i) {
			--_data[i];
		}
		return *this;
	}
	mexMatrixT operator--(int) //suffix
	{
		mexMatrixT out(*this);
		for (size_t i = 0; i < numel(); ++i) {
			--_data[i];
		}
		return out;
	}

	template<typename M> mexMatrixT& operator+=(const mexMatrixT<M>& b) {
		if (numel() != b.numel()) mexErrMsgTxt("mexMatrixT+=mexMatrixT<M>: matricies must have same number of elements");
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] += (T)(b._data[i]);
		}
		return *this;
	}
	template<typename M> mexMatrixT& operator-=(const mexMatrixT<M>& b) {
		if (numel() != b.numel()) mexErrMsgTxt("mexMatrixT-=mexMatrixT<M>: matricies must have same number of elements");
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] -= (T)(b._data[i]);
		}
		return *this;
	}
	template<typename M> mexMatrixT& operator*=(const mexMatrixT<M>&); //not defined
	template<typename M> mexMatrixT& operator/=(const mexMatrixT<M>&); //not defined
	template<typename M> mexMatrixT operator*(const mexMatrixT<M>&); //not defined
	template<typename M> mexMatrixT operator/(const mexMatrixT<M>&); //not defined

	mexMatrixT& operator+=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] += b;
		}
		return *this;
	}
	mexMatrixT& operator-=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] -= b;
		}
		return *this;
	}
	mexMatrixT& operator*=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] *= b;
		}
		return *this;
	}
	mexMatrixT& operator/=(const T& b) {
		for (size_t i = 0; i < numel(); ++i) {
			_data[i] /= b;
		}
		return *this;
	}

	mexMatrixT<T> operator+(const mexMatrixT<T>& b) //same type add
	{
		if (numel() != b.numel()) mexErrMsgTxt("mexMatrixT<T>+mexMatrixT<T>: matricies must have same number of elements");
		mexMatrixT<T> out(*this);
		out += b;
		return out;
	}
	mexMatrixT<T> operator-(const mexMatrixT<T>& b) //same type subtract
	{
		if (numel() != b.numel()) mexErrMsgTxt("mexMatrixT<T>-mexMatrixT<T>: matricies must have same number of elements");
		mexMatrixT<T> out(*this);
		out -= b;
		return out;
	}
	mexMatrixT<T> operator*(const T& v) {
		mexMatrixT<T> out(*this);
		out *= v;
		return out;
	}

	template<typename M> mexMatrixT<double> operator+(const mexMatrixT<M>& b) //different type add
	{
		if (numel() != b.numel()) mexErrMsgTxt("mexMatrixT<T>+mexMatrixT<M>: matricies must have same number of elements");
		mexMatrixT<double> out(*this);
		out += b;
		return out;
	}
	template<typename M> mexMatrixT<double> operator-(const mexMatrixT<M>& b) //different type subtract
	{
		if (numel() != b.numel()) mexErrMsgTxt("mexMatrixT<T>-mexMatrixT<M>: matricies must have same number of elements");
		mexMatrixT<double> out(*this);
		out -= b;
		return out;
	}

	mexMatrixT<T> operator+(const T& b) //same type
	{
		mexMatrixT<T> out(*this);
		out += b;
		return out;
	}
	template<typename M> mexMatrixT<double> operator+(const M& b) //dif type
	{
		mexMatrixT<double> out(*this);
		out += b;
		return out;
	}

	//dont use same type for -, might loose sign

	//dif type -
	template<typename M> mexMatrixT<double> operator-(const M& b) //dif type
	{
		mexMatrixT<double> out(*this);
		out -= b;
		return out;
	}
	//dif type *
	template<typename M> mexMatrixT<double> operator*(const M& b) {
		mexMatrixT<double> out(*this);
		out *= b;
		return out;
	}
	//dif type /
	template<typename M> mexMatrixT<double> operator/(const M& b) {
		mexMatrixT<double> out(*this);
		out /= b;
		return out;
	}

	template<typename V, typename M>
	friend mexMatrixT<double> operator-(const V&, const mexMatrixT<M>&);
	template<typename V, typename M>
	friend mexMatrixT<double> operator*(const V&, const mexMatrixT<M>&);

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


template<typename M>
void disp(const mexMatrixT<M>& m) {
	m.disp();
}

template<typename M>
size_t numel(const mexMatrixT<M>& m) {
	return m.numel();
}
