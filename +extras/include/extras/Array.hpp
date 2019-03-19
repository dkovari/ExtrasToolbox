/*
Array.hpp - Classes for storing 2D data arrays
*/
#pragma once
//#include <mex.h>
#include "ArrayBase.hpp"
#include <vector>
#include <stdexcept>
#include <cstring>
#include <string>

#include "numeric.hpp"

// if mex.h is included, mexPrintf is just an alias of printf
//if not included, include cstdio so that we can use printf()
#ifndef mex_typedefs_h
#include <cstdio>
#endif


namespace extras{

	///C++ Native extras::ArrayBase with self-managed memory.
	///with column-major data storage
	template<typename T> class Array: virtual public extras::ArrayBase<T> {
	protected:
		T* _data = nullptr; ///< pointer to data
		std::vector<size_t> _dim{0,0}; ///< dimensions of the data
		size_t _rows = 0; ///< number of rows (just the _dim[0])
		size_t _cols = 0; ///< number of cols assiming data is a matrix (i.e. product of _dim[1]*_dim[2]_*...)
		std::vector<size_t> _dimOffset{1,0}; ///< pre-computed offset index for each dimension in _data, _dimOffset[0]=1 _dimOffset[1]=_dim[0], _dimOffset[2] = _dim[0]*_dim[1]
		size_t _numel = 0; ///> pre-computed number of elements (prod of _dim)

		/// clear data from memory
		void freedata(){
			if(_data!=nullptr){
				free(_data);
				_data = nullptr;
			}
		}

		void mallocdata(size_t sz){
			_data = (T*)malloc(sz*sizeof(T));
			_numel = sz;
		}
		void callocdata(size_t sz){
			_data = (T*)calloc(sz, sizeof(T));
			_numel=sz;
		}
		void reallocdata(size_t sz){
			_data = (T*)realloc(_data, sz*sizeof(T));
			_numel=sz;
		}
		void recallocdata(size_t sz){
			_data = (T*)realloc(_data,sz*sizeof(T));
			if(sz>_numel){
				for(size_t n=_numel-1;n<sz;++n){
					_data[n] = 0;
				}
			}
			_numel=sz;
		}

		/// move to array from other array
		void moveFrom( extras::Array<T>& src){
			freedata();
			_data = src._data;

			_numel = src._numel;
			_rows = src._rows;
			_cols = src._cols;
			_dim = std::move(src._dim);
			_dimOffset = std::move(src._dimOffset);

			src._data = nullptr;
			src._rows = 0;
			src._cols = 0;
			src._numel = 0;
		}

		void copyFrom( const extras::Array<T>& src){
			freedata();
			mallocdata(src._numel);
			_dim = src._dim;
			_rows = src._rows;
			_cols = src._cols;
			_dimOffset = src._dimOffset;

			std::memcpy((void*)_data,(void*)src._data,src._numel*sizeof(T));
		}

		void computeOffset(const std::vector<size_t>& dim){
			_dimOffset.resize(dim.size());
			if(dim.empty()){
				return;
			}
			_dimOffset[0] = 1;
			for(size_t i=1;i<dim.size();++i){
				_dimOffset[i] = _dimOffset[i-1]*dim[i-1];
			}
		}

		void computeCols(const std::vector<size_t>& dim){
			if(dim.empty()){
				_cols = 0;
			}else if(dim.size()==1){
				_cols = 1;
			}else{
				_cols = dim[1];
				for(size_t n=2;n<dim.size();++n){
					_cols *= dim[n];
				}
			}
		}

		void copyFrom( const extras::ArrayBase<T>& src){
			freedata();
			mallocdata(src.numel());
			_dim = src.dims();
			computeOffset(_dim);
			_rows = _dim[0];
			computeCols(_dim);

			std::memcpy((void*)_data,(void*)src.getdata(),_numel*sizeof(T));
		}
		template<typename M> void copyFrom(const extras::ArrayBase<M>& src){
			freedata();
			mallocdata(src.numel());
			_dim = src.dims();
			computeOffset(_dim);
			_rows = _dim[0];
			computeCols(_dim);

			// copy by element
			for(size_t n=0;n<_numel;++n){
				_data[n] = src[n];
			}
		}

	public:
		virtual size_t numel() const {return _numel;} ///< number of elements
    	virtual bool isempty() const {return _numel==0;} ///< is array empty
    	virtual size_t nRows() const {return _rows;} ///< number of rows
    	virtual size_t nCols() const {return _cols;} ///< number of columns

		virtual std::vector<size_t> dims() const {return _dim;} ///< returns dimensions of the data
    	virtual size_t ndims() const {return _dim.size();} ///< number of dimensions

    	virtual const T* getdata() const {return _data;} ///< get pointer to raw data array
    	virtual T* getdata() {return _data;} ///< get pointer to raw data array
    	
		
		//! returns index corresponding to subscript
		virtual size_t sub2ind(std::vector<size_t> subs) const {
			size_t idx = subs[0];
			for (size_t n = 1; n < subs.size(); ++n) {
				if (subs[n] == 0) { //ignore singleton dimensions
					continue;
				}
				if (n >= _dim.size()) {
					throw("Array::sub2ind(): subs longer than array dim and sub[n]!=0")
				}
				idx += _dimOffset[n] * subs[n];
			}
		}


		virtual T& operator[](size_t index) {
			using namespace std;
			if(index>=_numel){
				throw(runtime_error(string("Array::operator[")+to_string(index)+string("] index exceeds numel=")+to_string(_numel)));
			}
			return _data[index];
		} ///< set n-th element
    	virtual const T& operator[](size_t index) const {
			using namespace std;
			if(index>=_numel){
				throw(runtime_error(string("Array::operator[")+to_string(index)+string("] index exceeds numel=")+to_string(_numel)));
			}
			return _data[index];
		} ///< get n-th element
    	virtual T& operator()(size_t index) {return (*this)[index];} ///< set n-th element
    	virtual const T& operator()(size_t index) const {return (*this)[index];} ///< get n-th element
    	virtual T& operator()(size_t row, size_t col) {return (*this)[row+col*_rows];} ///< set element [m,n]
    	virtual const T& operator()(size_t row, size_t col) const {return (*this)[row+col*_rows];} ///< get element [m,n]

		///< set element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual T& operator()(const std::vector<size_t>& el){return (*this)[sub2ind(el)];}

		///< get element at coordinate [x,y,z,...] specified by the vector elementCoord
    	virtual const T& operator()(const std::vector<size_t>& el) const{return (*this)[sub2ind(el)];}

		///< resize to hold n elements, keep old data, new data set to zeros
    	virtual void resize(size_t numel){
			recallocdata(numel);
			_rows = numel;
			_cols = 1;
			_dim = {numel,1};
			_dimOffset = {1,numel};
		}

		///< resize to new size
		virtual void resize(const std::vector<size_t>& dim){
			recallocdata(prod(dim));
			_dim = dim;
			_rows = dim[0];
			computeCols(dim);
			computeOffset(dim);
		}
    	virtual void resize(size_t nRows, size_t nCols){resize({nRows,nCols});}		///< resize to hold MxN elements, keep old data, new data set to zeros

		///< resize to hold n elements, discard original data
    	virtual void resize_nocpy(size_t numel){
			mallocdata(numel);
			_rows = numel;
			_cols = 1;
			_dim = {numel,1};
			_dimOffset = {1,numel};
		}

		///< resize to n elements, discard original data
		virtual void resize_nocpy(const std::vector<size_t>& dim){
			mallocdata(prod(dim));
			_dim = dim;
			_rows = dim[0];
			computeCols(dim);
			computeOffset(dim);
		}
    	virtual void resize_nocpy(size_t nRows, size_t nCols){ resize_nocpy({nRows,nCols});}///< resize to hold MxN elements, discard original data

		///< resize to MxNx... elements, set all elements to zero
		virtual void resize_clear(const std::vector<size_t>& dim){
			callocdata(prod(dim));
			_dim = dim;
			_rows = dim[0];
			computeCols(dim);
			computeOffset(dim);
		}
    	virtual void resize_clear(size_t rows, size_t cols){resize_clear({rows,cols});}///< resize to hold MxN elements, set all elements to zero

		///////////////////
		// Destructor
		virtual ~Array(){freedata();}

		//////////////////
		// Constructors

		/// Construct from no args, create empty array
		Array(){};//NOTHIG TO DO

		/// construct from vector specifying dims
		Array(const std::vector<size_t>& dim, bool SET_TO_ZERO=false){
			if(SET_TO_ZERO){
				resize_clear(dim);
			}else{
				resize_nocpy(dim);
			}
		}

		///construct matrix with size [rows x cols]
		Array(size_t rows, size_t cols, bool SET_TO_ZERO=false){
			if(SET_TO_ZERO){
				resize_clear(rows,cols);
			}else{
				resize_nocpy(rows,cols);
			}
		}

		/// construct vector size:[numel x 1]
		Array(size_t numel){
			resize_nocpy(numel);
		}

		/// copy constructor
		template<typename M> Array(const extras::ArrayBase<M>& src){ copyFrom(src);}

		/// copy assignment
		template<typename M> Array& operator=(const extras::ArrayBase<M>& src){ copyFrom(src); return *this;}

		/// move constructor
		Array(extras::Array<T>&& src){moveFrom(src);}

		/// move assignment
		Array& operator=(extras::Array<T>&& src){moveFrom(src);}

		/// Assign from vector, create size:[vec.size x 1]
		template<typename M> Array& operator=(const std::vector<M>& src){
			resize_nocpy(src.size());
			for(size_t n=0;n<src.size();++n){
				(*this)[n] = src[n];
			}
			return *this;
		}

		/// display the matrix
		void disp() const{
			printf("Array<%s> [", typeid(T).name());
			for(size_t n=0;n<ndims()-1;++n){
				printf("%zu x ",dims()[n]);
			}
			printf("%zu]\n",dims()[ndims()-1]);
			if(ndims()<=2){
				for(size_t r=0;r<nRows();++r){
					for(size_t c=0;c<nCols();++c){
						printf("\t%g",(double)(*this)(r,c));
					}
					printf("\n");
				}
			}
		}

		/////////////////////////////
		//

		//! Concatenate with generic array
		template <typename M>
		Array& concatenate(const ArrayBase<M>& b, size_t dim) {

			////////////////////////////
			// Compute new size

			auto thisSz = size();
			auto thatSz = end_obj.size();
			size_t thisSz_len = thisSz.size();
			size_t thatSz_len = thatSz.size();

			size_t maxDim = std::max(thisSz.size(), thatSz.size());


			// Loop over array dimensions and determine if sizes are compatible
			thisSz.resize(maxDim);
			thatSz.resize(maxDim);

			for (size_t j = 0; j < maxDim; j++) {

				if (j >= thisSz_len) {
					thisSz[j] = 1;
				}

				if (j >= thatSz_len) {
					thatSz[j] = 1;
				}

				if (j == dim) {
					continue;
				}

				if (thisSz[j] != thatSz[j]) {
					throw(std::runtime_error("incompatible sizes for concatenate"));
				}
			}

			auto newSz = thisSz;
			newSz[dim] = thisSz[dim] + thatSz[dim];

			//////////////////////////////////////////////////
			// Create Temporary array to hold new data

			Array<T> newArray(newSz);

			//////////////////
			// Loop over dimension dim and copy data for all other dimensions

			size_t new_dim = 0; //accumulator for current position in newPtr
			for (size_t d = 0; d < thisSz[dim]; d++) { //first loop over mxptr elements
				//loop over other dims
				for (size_t odim = 0; odim < newSz.size(); odim++) {
					if (odim == dim) { //nothing to do for dim
						continue;
					}
					//loop over indecies of odim and copy
					for (size_t od = 0; od < thisSz[odim]; od++) {
						std::vector<size_t> subs(newSz.size(), 0.0); //create subscript array with all zeros
						subs[odim] = od; //index for dimension being copied
						subs[dim] = d; //index for dimension being concatenated

						newArray(subs) = (*this)(subs); //copy values

					}
				}
				new_dim++;
			}

			for (size_t d = 0; d < thatSz[dim]; d++) { //second loop over end_obj elements
				//loop over other dims
				for (size_t odim = 0; odim < newSz.size(); odim++) {
					if (odim == dim) { //nothing to do for dim
						continue;
					}
					//loop over indecies of odim and copy
					for (size_t od = 0; od < thatSz[odim]; od++) {
						std::vector<size_t> subs(newSz.size(), 0.0); //create subscript array with all zeros
						subs[odim] = od; //index for dimension being copied
						subs[dim] = d; //index for dimension being concatenated

						auto val = b(subs); //make tmp copy of data

						subs[dim] = new_dim; //change the sub, since newArray has different dim
						newArray(subs) = val; //set value
					}
				}
				new_dim++;
			}

			moveFrom(newArray);

			return *this;
		}


	};
}
