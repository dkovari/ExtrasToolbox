/*
ArrayBase.hpp - Generic Interface Class for storing data arrays
*/
#pragma once

#include<vector>
#include <stdexcept>

namespace extras {
    ///ArrayBase Interface Class.
    ///Array classes should derive from this class and implement the public interface methods
    template<typename T> class ArrayBase {
    public:
    	virtual ~ArrayBase(){};

    	virtual size_t numel() const = 0; ///< number of elements
    	virtual bool isempty() const = 0; ///< is array empty
    	virtual size_t nRows() const = 0; ///< number of rows
    	virtual size_t nCols() const = 0; ///< number of columns

		virtual std::vector<size_t> dims() const = 0; ///< returns dimensions of the data
		virtual size_t ndims() const = 0; ///< number of dimensions

    	virtual const T* getdata() const = 0; ///< get pointer to raw data array
    	virtual T* getdata() = 0; ///< get pointer to raw data array

    	virtual T& operator[](size_t index) = 0; ///< set n-th element
    	virtual const T& operator[](size_t index) const = 0; ///< get n-th element

    	virtual T& operator()(size_t index) = 0; ///< set n-th element
    	virtual const T& operator()(size_t index) const = 0; ///< get n-th element

    	virtual T& operator()(size_t row, size_t col) = 0; ///< set element [m,n]
    	virtual const T& operator()(size_t row, size_t col) const = 0; ///< get element [m,n]

		virtual T& operator()(const std::vector<size_t>& elementCoord) = 0; ///< set element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual const T& operator()(const std::vector<size_t>& elementCoord) const = 0; ///< get element at coordinate [x,y,z,...] specified by the vector elementCoord

    	virtual void resize(size_t numel) = 0; ///< resize to hold n elements, keep old data, new data set to zeros
    	virtual void resize(size_t nRows, size_t nCols) = 0; ///< resize to hold MxN elements, keep old data, new data set to zeros
		virtual void resize(const std::vector<size_t>& dim) = 0; ///< resize to new size

    	virtual void resize_nocpy(size_t numel) = 0; ///< resize to hold n elements, discard original data
    	virtual void resize_nocpy(size_t nRows, size_t nCols) = 0;///< resize to hold MxN elements, discard original data
		virtual void resize_nocpy(const std::vector<size_t>& dim) = 0; ///< resize to n elements, discard original data

    	virtual void resize_clear(size_t rows, size_t cols) = 0;///< resize to hold MxN elements, set all elements to zero
		virtual void resize_clear(const std::vector<size_t>& dim) = 0;///< resize to MxNx... elements, set all elements to zero

        ///////////////
        // Implemented Here

        virtual size_t elementsize() const { return sizeof(T); } ///< element size in bytes
        virtual ArrayBase& operator+=(double val){
    		for(size_t n=0;n<numel();++n){
    			(*this)[n] += val;
    		}
    		return *this;
    	}
    	virtual ArrayBase& operator-=(double val){
    		for(size_t n=0;n<numel();++n){
    			(*this)[n] -= val;
    		}
    		return *this;
    	}
    	virtual ArrayBase& operator/=(double val){
    		for(size_t n=0;n<numel();++n){
    			(*this)[n] /= val;
    		}
    		return *this;
    	}
    	virtual ArrayBase& operator*=(double val){
    		for(size_t n=0;n<numel();++n){
    			(*this)[n] *= val;
    		}
    		return *this;
    	}

		//assign from scalar
		ArrayBase& operator=(T val) {
			this->resize_nocpy(1);
			(*this)[0] = val;
			return *this;
		}

		//assign from vector
		template<typename M> ArrayBase& operator=(const std::vector<M>& val) {
			this->resize_nocpy(val.size());
			size_t n = 0;
			for (auto& v : val) {
				(*this)[n] = v;
				++n;
			}
			return *this;
		}

		 /// display the matrix
		virtual	void disp() const {
			printf("ArrayBase<%s> [", typeid(T).name());
			for (size_t n = 0; n<ndims() - 1; ++n) {
				printf("%zu x ", dims()[n]);
			}
			printf("%zu]\n", dims()[ndims() - 1]);
			if (ndims() <= 2) {
				for (size_t r = 0; r<nRows(); ++r) {
					for (size_t c = 0; c<nCols(); ++c) {
						printf("\t%g", (double)(*this)(r, c));
					}
					printf("\n");
				}
			}
		}

    };
}

template<typename T> void disp(const extras::ArrayBase<T>& A) {A.disp();}
template<typename T> size_t numel(const extras::ArrayBase<T>& obj){return obj.numel();}
template<typename T> bool isempty(const extras::ArrayBase<T>& obj){return obj.isempty();}
template<typename T> size_t nRows(const extras::ArrayBase<T>& obj){return obj.nRows();}
template<typename T> size_t nCols(const extras::ArrayBase<T>& obj){return obj.nCols();}
template<typename T> size_t ndims(const extras::ArrayBase<T>& obj){return obj.ndims();}
template<typename T> std::vector<size_t> dims(const extras::ArrayBase<T>& obj){ return obj.dims();}
