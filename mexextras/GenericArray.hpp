/*
GenericArray.hpp - Classes for storing 2D data arrays
*/
#pragma once
//#include <mex.h>
#include<vector>
#include <stdexcept>

template <typename T> T cumprod(const std::vector<T> vec){
	if(vec.empty()){
		return 0;
	}
	T out = 1;
	for(size_t n=0;n<vec.size();++n){
		out*=vec[n];
	}
	return out;
}

///TypedArray Interface Class.
///Array classes should derive from this class and implement the public interface methods
template<typename T> class TypedArray {
public:
	virtual ~TypedArray(){};

	virtual size_t numel() const = 0; ///< number of elements
	virtual bool isempty() const = 0; ///< is array empty
	virtual size_t nRows() const = 0; ///< number of rows
	virtual size_t nCols() const = 0; ///< number of columns
	virtual size_t elementsize() const { return sizeof(T); } ///< element size in bytes

	virtual const T* getdata() const = 0; ///< get pointer to raw data array
	virtual T* getdata() = 0; ///< get pointer to raw data array
	virtual T& operator[](size_t index) = 0; ///< set n-th element
	virtual const T& operator[](size_t index) const = 0; ///< get n-th element
	virtual T& operator()(size_t index) = 0; ///< set n-th element
	virtual const T& operator()(size_t index) const = 0; ///< get n-th element
	virtual T& operator()(size_t row, size_t col) = 0; ///< set element [m,n]
	virtual const T& operator()(size_t row, size_t col) const = 0; ///< get element [m,n]

	virtual void resize(size_t numel) = 0; ///< resize to hold n elements, keep old data, new data set to zeros
	virtual void resize(size_t nRows, size_t nCols) = 0; ///< resize to hold MxN elements, keep old data, new data set to zeros
	virtual void resize_nocpy(size_t numel) = 0; ///< resize to hold n elements, discard original data
	virtual void resize_nocpy(size_t nRows, size_t nCols) = 0;///< resize to hold MxN elements, discard original data
	virtual void resize_clear(size_t rows, size_t cols) = 0;///< resize to hold MxN elements, set all elements to zero

	virtual TypedArray& operator+=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] += val;
		}
		return *this;
	}
	virtual TypedArray& operator-=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] -= val;
		}
		return *this;
	}
	virtual TypedArray& operator/=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] /= val;
		}
		return *this;
	}
	virtual TypedArray& operator*=(double val){
		for(size_t n=0;n<numel();++n){
			(*this)[n] *= val;
		}
		return *this;
	}

	//////////////////
	// ND Array support
	virtual std::vector<size_t> dims() const = 0; ///< returns dimensions of the data
	virtual size_t ndims() const = 0; ///< number of dimensions

	virtual T& operator()(const std::vector<size_t>& elementCoord) = 0; ///< set element at coordinate [x,y,z,...] specified by the vector elementCoord
	virtual const T& operator()(const std::vector<size_t>& elementCoord) const = 0; ///< get element at coordinate [x,y,z,...] specified by the vector elementCoord

	virtual void resize(const std::vector<size_t>& dim) = 0; ///< resize to new size
	virtual void resize_nocpy(const std::vector<size_t>& dim) = 0; ///< resize to n elements, discard original data
	virtual void resize_clear(const std::vector<size_t>& dim) = 0;///< resize to MxNx... elements, set all elements to zero
};

template<typename T> size_t numel(const TypedArray<T>& obj){return obj.numel();}
template<typename T> bool isempty(const TypedArray<T>& obj){return obj.isempty();}
template<typename T> size_t nRows(const TypedArray<T>& obj){return obj.nRows();}
template<typename T> size_t nCols(const TypedArray<T>& obj){return obj.nCols();}
template<typename T> size_t ndims(const TypedArray<T>& obj){return obj.ndims();}
template<typename T> std::vector<size_t> dims(const TypedArray<T>& obj){ return obj.dims();}

///C++ Native TypedArray with self-managed memory.
///with column-major data storage
template<typename T> class NativeArray: public TypedArray<T> {
protected:
	T* _data=nullptr;
	bool _managedata = true;
	size_t _numel = 0; //total number of elements
	size_t _nRows = 0; //number of rows
	size_t _nCols = 0; //(number of elements)/(nRows)
	std::vector<size_t> _dims; //vector of dimensions
	std::vector<size_t> _pagesz; //array containing memory page offset sizes

	/// called whenever dims are changed
	// does not change memory
	virtual void updateDims(const std::vector<size_t>& dim){
		bool sameDim = true;
		if(_dims.size()!=dim.size()){
			sameDim = false;
		}
		for(size_t n=0;(n<dim.size()&&sameDim);++n){
			if(_dims[n]!=dim[n]){
				sameDim = false;
			}
		}
		if(!sameDim){
			computePagesz(dim);

			if(dim.empty()){
				_numel = 0;
				_nRows = 0;
				_nCols = 0;
			}
			else{
				_numel = _pagesz.back()*dim.back();
				_nRows = dim[0];
				_nCols = _numel/_nRows;
			}
			_dims = dim;
		}
	}

	virtual void computePagesz(const std::vector<size_t>& dim){
		_pagesz.resize(dim.size());
		if(!dim.empty()){
			_pagesz[0] = 1;
			for(size_t n=1;n<dim.size();++n){
				_pagesz[n] = _pagesz[n-1]*dim[n-1];
			}
		}
	}

	virtual void freedata() {
		if (_managedata && _data != nullptr) {
			free(_data);
			_data = nullptr;
			_numel = 0;
			_nRows = 0;
			_nCols = 0;
			_dims.clear();
			_pagesz.clear();
			_managedata = true;
		}
	}

	virtual void mallocdata(const std::vector<size_t>& dim){
		freedata();
		updateDims(dim);
		if(_numel>0){
			_data = (T*)malloc(_numel*sizeof(T));
			_managedata = true;
		}
	}
	virtual void mallocdata(size_t rows, size_t cols) {
		mallocdata({rows,cols});
	}
	virtual void mallocdata(size_t numel) {
		mallocdata(numel, 1);
	}
	virtual void mallocdata() {
		std::vector<size_t> dim = _dims;
		mallocdata(dim);
	}


	virtual void callocdata(const std::vector<size_t>& dim){
		freedata();
		updateDims(dim);
		if(_numel>0){
			_data = (T*)calloc(_numel,sizeof(T));
			_managedata = true;
		}
	}
	virtual void callocdata(size_t rows, size_t cols) {
		freedata();
		callocdata({rows,cols});
	}
	virtual void callocdata(size_t numel) {
		callocdata(numel, 1);
	}
	virtual void callocdata() {
		std::vector<size_t> dim = _dims;
		callocdata(dim);
	}

	virtual void reallocdata(const std::vector<size_t>& dim){

		if(_numel==cumprod(dim)){ //size not changing, just update dims
			updateDims(dim);
		}
		else if(!_managedata){ //not managing data, copy
			size_t new_numel = cumprod(dim);
			T* newdata = (T*)malloc(new_numel*sizeof(T));
			if(new_numel<_numel){
				memcpy(newdata,_data,new_numel*sizeof(T));
			}else{
				memcpy(newdata,_data,_numel*sizeof(T));
			}
			_managedata = true;
			updateDims(dim);
		}
		else { //managing data, realloc
			updateDims(dim);
			if (_data != nullptr) {
				_data = (T*)realloc(_data, _numel * sizeof(T));
			}
			else
			{
				_data = (T*)malloc(_numel * sizeof(T));
			}
		}
	}
	virtual void reallocdata(size_t rows, size_t cols) {
		reallocdata({rows,cols});
	}
	virtual void reallocdata(size_t numel) {
		reallocdata(numel, 1);
	}
	virtual void reallocdata() {
		std::vector<size_t> dim = _dims;
		reallocdata(dim);
	}

	///Copy from TypedArray with same storage type
	virtual void copyFrom(const TypedArray<T>& other){
		freedata();
		updateDims(other.dims());
		mallocdata();
		memcpy(_data, other.getdata(), _numel*sizeof(T));
	}

	///Copy from TypedArray with different storage type, perform type conversion
	template <typename M> void copyFrom(const TypedArray<M>& other){
		freedata();
		updateDims(other.dims());
		mallocdata();

		//copy value-by-value
		for(size_t n=0;n<_numel;++n){
			//mexPrintf("other[%d]=%g\n",n,other[n]);
			(*this)[n] = (T)(other[n]);
			//mexPrintf("this[%d]=%g\n",n,(*this)[n]);
		}
	}

	virtual void moveFrom(NativeArray<T>& other) {
		freedata();
		_data = other._data;
		_numel = other._numel;
		_nRows = other._nRows;
		_nCols = other._nCols;
		_dims = other._dims;
		_pagesz = other._pagesz;
		_managedata = other._managedata;

		other._managedata = false;
	}
public:
	virtual std::vector<size_t> dims() const{return _dims;} ///< returns dimensions of the data
	virtual size_t ndims() const {return _dims.size();} ///< number of dimensions

	virtual size_t numel() const { return _numel; } ///< number of elements in Array
	virtual bool isempty() const {return _numel == 0;} ///< true if array is empty
	virtual size_t nRows() const {return _nRows;}///< number of rows in the data

	///If array has more than 2 dim, nCols is the total product of dims 2:end
	//otherwise it is simple the number of columns
	virtual size_t nCols() const {return _nCols;}

	virtual const T* getdata() const { return _data; } ///< return pointer to data array
	virtual T* getdata() { return _data; } ///< return pointer to data array

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

	virtual T& operator()(const std::vector<size_t>& dim){
		if(dim.empty()){
			throw(std::runtime_error("NativeArray(): dim is empty"));
		}
		if(dim.size()>_dims.size()){
			throw(std::runtime_error("NativeArray(): dim is too long"));
		}

		size_t index = dim[0];
		for(size_t n=1;n<dim.size();++n){
			index+= dim[n]*_pagesz[n];
		}

		return (*this)[index];
	}
	virtual const T& operator()(const std::vector<size_t>& dim) const{
		if(dim.empty()){
			throw(std::runtime_error("NativeArray(): dim is empty"));
		}
		if(dim.size()>_dims.size()){
			throw(std::runtime_error("NativeArray(): dim is too long"));
		}

		size_t index = dim[0];
		for(size_t n=1;n<dim.size();++n){
			index+= dim[n]*_pagesz[n];
		}

		return (*this)[index];
	}

	virtual void resize(size_t numel) {reallocdata(numel);}
	virtual void resize(size_t rows, size_t cols) {reallocdata(rows, cols);}
	virtual void resize(const std::vector<size_t>& dim){reallocdata(dim);}

	///resize the array but don't copy values
	// the contained values will be undefined
	virtual void resize_nocpy(size_t numel) {mallocdata(numel);}
	virtual void resize_nocpy(size_t rows, size_t cols) {mallocdata(rows, cols);}
	virtual void resize_nocpy(const std::vector<size_t>& dim){mallocdata(dim);}

	///resize the array and set all elements to zeros
	virtual void resize_clear(size_t rows, size_t cols) {callocdata(rows, cols);}
	virtual void resize_clear(const std::vector<size_t>& dim){callocdata(dim);}


	// Destructor
	~NativeArray() {freedata();}

	//Constructors
	//---------------------------

	NativeArray() {
		_data = nullptr;
		_managedata = true;
		_numel = 0;
		_nRows = 0;
		_nCols = 0;
		_dims = {};
		_pagesz = {};
	}

	NativeArray( size_t numel){mallocdata(numel);}
	NativeArray(size_t rows, size_t cols){mallocdata(rows,cols);}
	NativeArray(size_t rows,size_t cols, bool SetZeros){
		if(SetZeros){
			callocdata(rows,cols);
		}
		else{
			mallocdata(rows,cols);
		}
	}
	NativeArray(const std::vector<size_t>& dim, bool setZeros = false){
		if(setZeros){
			callocdata(dim);
		}
		else{
			mallocdata(dim);
		}
	}

	NativeArray(T* data, size_t numel) {
		_data = data;
		_managedata = false;
		_numel = numel;
		_nRows = numel;
		_nCols = 1;
		_dims = {numel};
		_pagesz = {1};
	}

	NativeArray(const T* data, size_t numel) {
		_data = nullptr;
		_managedata = true;
		_numel = numel;
		_nRows = numel;
		_nCols = 1;
		_dims = {numel};
		_pagesz = {1};
		mallocdata();
		memcpy(_data, data, _numel * sizeof(T));
	}

	NativeArray(T* data, size_t nRows, size_t nCols) {
		_data = data;
		_managedata = false;
		_numel = nRows*nCols;
		_nRows = nRows;
		_nCols = nCols;
		_dims = {nRows,nCols};
		_pagesz = {1,nRows};

	}

	NativeArray(const T* data, size_t nRows, size_t nCols) {
		_data = nullptr;
		_managedata = true;
		_numel = nRows*nCols;
		_nRows = nRows;
		_nCols = nCols;
		_dims = {nRows,nCols};
		_pagesz = {1,nRows};
		mallocdata();
		memcpy(_data, data, _numel * sizeof(T));
	}

	NativeArray(T* data, const std::vector<size_t>& dim){
		_data = data;
		_managedata = false;
		updateDims(dim);
	}
	NativeArray(const T* data, const std::vector<size_t>& dim){
		_data = nullptr;
		_managedata = true;
		updateDims(dim);
		mallocdata();
		memcpy(_data, data, _numel * sizeof(T));
	}

	template <typename M> NativeArray(const TypedArray<M>& other) {
		//mexPrintf("Construct from other <M>\n");
		this->copyFrom(other);
	}

	template <typename M> NativeArray& operator=(const TypedArray<M>& other) {
		this->copyFrom(other);
		return *this;
	}

	NativeArray(NativeArray&& other) {
		this->moveFrom(other);
	}
	NativeArray& operator=(NativeArray&& other) {
		this->moveFrom(other);
		return *this;
	}

};
