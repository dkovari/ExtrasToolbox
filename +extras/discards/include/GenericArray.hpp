#pragma once

#include<vector>
#include <stdexcept>

//Interface to TypedArray
template<typename T> class TypedArrayI {
public:
	virtual ~TypedArrayI(){};
	
	virtual size_t numel() const = 0;
	virtual bool isempty() const = 0;
	virtual size_t nRows() const = 0;
	virtual size_t nCols() const = 0;
	virtual size_t elementsize() const { return sizeof(T); } //element size in bytes

	//virtual size_t ndims() const = 0;
	//virtual const size_t* dims() = 0;

	virtual const T* getdata() const = 0;
	virtual T* getdata() = 0;
	virtual T& operator[](size_t index) = 0;
	virtual const T& operator[](size_t index) const = 0;
	virtual T& operator()(size_t index) = 0;
	virtual const T& operator()(size_t index) const = 0;
	virtual T& operator()(size_t row, size_t col) = 0;
	virtual const T& operator()(size_t row, size_t col) const = 0;

	virtual void resize(size_t numel) = 0;
	virtual void resize(size_t nRows, size_t nCols) = 0;
	virtual void resize_nocpy(size_t numel) = 0;
	virtual void resize_nocpy(size_t nRows, size_t nCols) = 0;
	virtual void resize_clear(size_t rows, size_t cols) = 0;

	virtual TypedArrayI& operator+=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] += val;
		}
		return *this;
	}
	virtual TypedArrayI& operator-=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] -= val;
		}
		return *this;
	}
	virtual TypedArrayI& operator/=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] /= val;
		}
		return *this;
	}
	virtual TypedArrayI& operator*=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] *= val;
		}
		return *this;
	}
};

template<typename T> size_t numel(const TypedArrayI<T>& obj){return obj.numel();}
template<typename T> bool isempty(const TypedArrayI<T>& obj){return obj.isempty();}
template<typename T> size_t nRows(const TypedArrayI<T>& obj){return obj.nRows();}
template<typename T> size_t nCols(const TypedArrayI<T>& obj){return obj.nCols();}

//C++ Native TypedArray with self-managed memory
//with column-major data storage
template<typename T> class NativeArray: public TypedArrayI<T> {
protected:
	T* _data=nullptr;
	bool _managedata = true;
	size_t _numel = 0;
	size_t _nRows = 0;
	size_t _nCols = 0;
	//std::vector<size_t> _dims;

	virtual void freedata() {
		if (_managedata && _data != nullptr) {
			free(_data);
			_data = nullptr;
			_numel = 0;
			_nRows = 0;
			_nCols = 0;
			_managedata = true;
		}
	}

	virtual void mallocdata(size_t rows, size_t cols) {
		freedata();
		if (rows*cols > 0) {
			_numel = rows*cols;
			_data = (T*)malloc(_numel * sizeof(T));
			_managedata = true;
			_nRows = rows;
			_nCols = cols;
		}
	}
	virtual void mallocdata(size_t numel) {
		mallocdata(numel, 1);
	}
	virtual void mallocdata() {
		size_t m = _nRows;
		size_t n = _nCols;
		mallocdata(m, n);
	}


	virtual void callocdata(size_t rows, size_t cols) {
		freedata();
		if (rows*cols > 0) {
			_numel = rows*cols;
			_data = (T*)calloc(_numel, sizeof(T));//(T*)malloc(_numel * sizeof(T));
			_managedata = true;
			_nRows = rows;
			_nCols = cols;
		}
	}
	virtual void callocdata(size_t numel) {
		callocdata(numel, 1);
	}
	virtual void callocdata() {
		size_t m = _nRows;
		size_t n = _nCols;
		callocdata(m, n);
	}

	virtual void reallocdata(size_t rows, size_t cols) {

		if (rows*cols == _numel) {//no need to realloc
			_nRows = rows;
			_nCols = cols;
		}
		else if (!_managedata) { //not managing data, copy
			T* newdata = (T*)malloc(rows*cols * sizeof(T));
			memcpy(newdata, _data, rows*cols * sizeof(T));
			_managedata = true;
			_data = newdata;
			_nRows = rows;
			_nCols = cols;
			_numel = rows*cols;
		}
		else { //managing data, realloc
			if (_data != nullptr) {
				_data = (T*)realloc(_data, rows*cols * sizeof(T));
			}
			else
			{
				_data = (T*)malloc(rows*cols * sizeof(T));
			}
			_nRows = rows;
			_nCols = cols;
			_numel = rows*cols;
		}

		freedata();
		if (rows*cols > 0) {
			_numel = rows*cols;
			_data = (T*)calloc(_numel, sizeof(T));//(T*)malloc(_numel * sizeof(T));
			_managedata = true;
			_nRows = rows;
			_nCols = cols;
		}
	}
	virtual void reallocdata(size_t numel) {
		reallocdata(numel, 1);
	}
	virtual void reallocdata() {
		size_t m = _nRows;
		size_t n = _nCols;
		reallocdata(m, n);
	}


public:
	virtual size_t numel() const { return _numel; } //number of elements in Array
	virtual bool isempty() const {return _numel == 0;} //true if array is empty
	//number of rows in the data
	virtual size_t nRows() const
	{
		return _nRows;
	}
	//If array has more than 2 dim, nCols is the total product of dims 2:end
	//otherwise it is simple the number of columns
	virtual size_t nCols() const
	{
		return _nCols;
	}

	//virtual size_t ndims() const = 0;
	//virtual const size_t* dims() = 0;

	virtual const T* getdata() const { return _data; } //return pointer to data array
	virtual T* getdata() { return _data; } //return pointer to data array

	virtual T& operator[](size_t index) {
		if (index >= _numel) {
			throw(std::runtime_error("index exceeds number of elements"));
		}
		return _data[index];
	}
	virtual const T& operator[](size_t index) const {
		{
			if (index >= _numel) {
				throw(std::runtime_error("index exceeds number of elements"));
			}
			return _data[index];
		}
	}

	virtual T& operator()(size_t index) { return (*this)[index]; }
	virtual const T& operator()(size_t index) const { return (*this)[index]; }

	virtual T& operator()(size_t row, size_t col) {
		if (row >= _nRows) {
			throw(std::runtime_error("row exceeds number of rows"));
		}
		if(col >= _nCols) {
			throw(std::runtime_error("col exceeds number of columns"));
		}

		return _data[row + _nRows*col];
	}
	virtual const T& operator()(size_t row, size_t col) const {
		if (row >= _nRows) {
			throw(std::runtime_error("row exceeds number of rows"));
		}
		if (col >= _nCols) {
			throw(std::runtime_error("col exceeds number of columns"));
		}

		return _data[row + _nRows*col];
	}

	//virtual T& operator()(size_t row, size_t col...) = 0;
	//virtual const T& operator()(size_t row, size_t col...) const = 0;

	virtual void resize(size_t numel) {
		reallocdata(numel);
	}

	virtual void resize(size_t rows, size_t cols) {
		reallocdata(rows, cols);
	}

	//resize the array but don't copy values
	// the contained values will be undefined
	virtual void resize_nocpy(size_t numel) {
		mallocdata(numel);
	}
	//resize the array but don't copy values
	// the contained values will be undefined
	virtual void resize_nocpy(size_t rows, size_t cols) {
		mallocdata(rows, cols);
	}
	//resize the array and set all elements to zeros
	virtual void resize_clear(size_t rows, size_t cols) {
		callocdata(rows, cols);
	}


	// Destructor
	~NativeArray() {
		freedata();
	}

	//Constructors
	//---------------------------

	NativeArray() {
		_data = nullptr;
		_managedata = true;
		_numel = 0;
		_nRows = 0;
		_nCols = 0;
	}

	NativeArray( size_t numel){
		mallocdata(numel);
	}
	NativeArray(size_t rows, size_t cols){
		mallocdata(rows,cols);
	}
	NativeArray(size_t rows,size_t cols, bool SetZeros){
		if(SetZeros){
			callocdata(rows,cols);
		}
		else{
			mallocdata(rows,cols);
		}
	}

	NativeArray(T* data, size_t numel) {
		_data = data;
		_managedata = false;
		_numel = numel;
		_nRows = numel;
		_nCols = 1;
	}

	NativeArray(const T* data, size_t numel) {
		_data = nullptr;
		_managedata = true;
		_numel = numel;
		_nRows = numel;
		_nCols = 1;
		mallocdata();
		memcpy(_data, data, _numel * sizeof(T));
	}

	NativeArray(T* data, size_t nRows, size_t nCols) {
		_data = data;
		_managedata = false;
		_numel = nRows*nCols;
		_nRows = nRows;
		_nCols = nCols;
	}

	NativeArray(const T* data, size_t nRows, size_t nCols) {
		_data = nullptr;
		_managedata = true;
		_numel = nRows*nCols;
		_nRows = nRows;
		_nCols = nCols;
		mallocdata();
		memcpy(_data, data, _numel * sizeof(T));
	}

	virtual void copy(const NativeArray<T>& other){
		freedata();
		_numel = other._numel;
		_nRows = other._nRows;
		_nCols = other._nCols;
		mallocdata();

		memcpy(_data, other._data, _numel*sizeof(T));
	}

	virtual void move(NativeArray<T>&& other) {
		freedata();
		_data = other._data;
		_numel = other._numel;
		_nRows = other._nRows;
		_nCols = other._nCols;
		_managedata = other._managedata;

		other._managedata = false;
	}

	NativeArray(const NativeArray& other) {
		this->copy(other);
	}
	NativeArray& operator=(const NativeArray& other) {
		this->copy(other);
		return *this;
	}

	NativeArray(NativeArray&& other) {
		this->move(other);
	}
	NativeArray& operator=(NativeArray&& other) {
		this->move(other);
		return *this;
	}


};
