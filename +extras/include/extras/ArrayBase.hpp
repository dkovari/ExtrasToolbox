/*
ArrayBase.hpp - Generic Interface Class for storing data arrays
*/
#pragma once

#include <extras/stacktrace_error.hpp>
#include<vector>
#include <stdexcept>

namespace extras {
	using std::size_t;

	// enumeration of standard numeric value types
	// if mxClassID is defined then the varous mxXXX_CLASS define are valid candicates for conversion
	enum valueType {
		vt_undefined = 0,
#ifdef mex_typedefs_h //mxClassID defined
		vt_double = mxClassID::mxDOUBLE_CLASS,
		vt_float = mxClassID::mxSINGLE_CLASS,
		vt_int8 = mxClassID::mxINT8_CLASS,
		vt_uint8 = mxClassID::mxUINT8_CLASS,
		vt_int16 = mxClassID::mxINT16_CLASS,
		vt_uint16 = mxClassID::mxUINT16_CLASS,
		vt_int32 = mxClassID::mxINT32_CLASS,
		vt_uint32 = mxClassID::mxUINT32_CLASS,
		vt_int64 = mxClassID::mxINT64_CLASS,
		vt_uint64 = mxClassID::mxUINT64_CLASS,
		vt_bool = mxClassID::mxLOGICAL_CLASS
#else //mxClassID not defined
		vt_double = 6,
		vt_float = 7,
		vt_int8 = 8,
		vt_uint8 = 9,
		vt_int16 = 10,
		vt_uint16 = 11,
		vt_int32 = 12,
		vt_uint32 = 13,
		vt_int64 = 14,
		vt_uint64 = 15,
		vt_bool = 3
#endif //End #ifdef mxClassID
	};

#ifdef mex_typedefs_h
	valueType mxClassID2valueType(mxClassID cid) {
		switch (cid) {
		case mxDOUBLE_CLASS:
		case mxSINGLE_CLASS:
		case mxINT8_CLASS:
		case mxUINT8_CLASS:
		case mxINT16_CLASS:
		case mxUINT16_CLASS:
		case mxINT32_CLASS:
		case mxUINT32_CLASS:
		case mxINT64_CLASS:
		case mxUINT64_CLASS:
		case mxLOGICAL_CLASS:
			return static_cast<valueType>(cid);
		default:
			return vt_undefined;
		}
	}
#endif

	template<typename M>
	valueType type2valueType() {return valueType::vt_undefined;}
	template<> valueType type2valueType<double>() { return valueType::vt_double; };
	template<> valueType type2valueType<float>() { return valueType::vt_float; };
	template<> valueType type2valueType<int8_t>() { return valueType::vt_int8; };
	template<> valueType type2valueType<uint8_t>() { return valueType::vt_uint8; };
	template<> valueType type2valueType<int16_t>() { return valueType::vt_int16; };
	template<> valueType type2valueType<uint16_t>() { return valueType::vt_uint16; };
	template<> valueType type2valueType<int32_t>() { return valueType::vt_int32; };
	template<> valueType type2valueType<uint32_t>() { return valueType::vt_uint32; };
	template<> valueType type2valueType<int64_t>() { return valueType::vt_int64; };
	template<> valueType type2valueType<uint64_t>() { return valueType::vt_uint64; };

	//! DynamicTypeArrayBase
	//! Interface class for array with runtime specified type
	class DynamicTypeArrayBase {
	protected:
		valueType _ValueType = valueType::vt_undefined;
		void changeValueType(valueType vt) {
			if (vt == vt_undefined) {
				throw(extras::stacktrace_error("Cannot construct DynamicTypeArrayBase with valueType:vt_undefined"));
			}
			_ValueType = vt;
		}

#ifdef mex_typedefs_h
		void changeValueType(mxClassID ct) {
			valueType vt = mxClassID2valueType(ct);
			if (vt == vt_undefined) {
				throw(extras::stacktrace_error("Cannot construct DynamicTypeArrayBase with valueType:vt_undefined"));
			}
			_ValueType = vt;
		}
#endif

	public:

		valueType getValueType() const { return _ValueType; }

		virtual ~DynamicTypeArrayBase() {};
		DynamicTypeArrayBase(valueType vt) : _ValueType(vt) {
			if (_ValueType == vt_undefined) {
				throw(extras::stacktrace_error("Cannot construct DynamicTypeArrayBase with valueType:vt_undefined"));
			}
		}

		virtual size_t numel() const = 0; ///< number of elements
		virtual bool isempty() const { return numel() == 0; }
		virtual size_t nRows() const = 0; ///< number of rows
		virtual size_t nCols() const = 0; ///< number of columns

		virtual std::vector<size_t> dims() const = 0; ///< returns dimensions of the data
		virtual std::vector<size_t> size() const { return dims(); } ///< returns dimensions of the data
		virtual size_t ndims() const = 0; ///< number of dimensions

		virtual const void* untyped_data() const = 0; ///< get pointer to raw data array
		virtual void* untyped_data() = 0; ///< get pointer to raw data array

		//! get type-cast data
		//! returns pointer to data with specified type
		//! if template type does not match internal _ValueType an error is thrown
		template <typename T>
		const T* typed_data() const {
			if (type2valueType<T>() != _ValueType) {
				throw(extras::stacktrace_error("DynamicTypeArrayBase::getTypedData(): Template type does not match internal ValueType"));
			}
			return (const T*)untyped_data();
		}

		//! get type-cast data
		//! returns pointer to data with specified type
		//! if template type does not match internal _ValueType an error is thrown
		template <typename T>
		T* typed_data() {
			if (type2valueType<T>() != _ValueType) {
				throw(extras::stacktrace_error("DynamicTypeArrayBase::getTypedData(): Template type does not match internal ValueType"));
			}
			return (T*)untyped_data();
		}
	};


    ///ArrayBase Interface Class.
    ///Array classes should derive from this class and implement the public interface methods
    template<typename T> class ArrayBase: virtual public DynamicTypeArrayBase {
    public:
		ArrayBase() :DynamicTypeArrayBase(type2valueType<T>()) {};

    	virtual ~ArrayBase(){};

    	virtual const T* getdata() const = 0; ///< get pointer to raw data array
    	virtual T* getdata() = 0; ///< get pointer to raw data array

		//! returns index corresponding to subscript
		virtual size_t sub2ind(const std::vector<size_t>& subs) const = 0;

    	virtual T& operator[](size_t index) = 0; ///< set n-th element
    	virtual const T& operator[](size_t index) const = 0; ///< get n-th element
		const T& getElement(size_t index) const{return (*this)[index];}; //alias for operator[] const

    	virtual T& operator()(size_t index) = 0; ///< set n-th element
    	virtual const T& operator()(size_t index) const = 0; ///< get n-th element

    	virtual T& operator()(size_t row, size_t col) = 0; ///< set element [m,n]
    	virtual const T& operator()(size_t row, size_t col) const = 0; ///< get element [m,n]
		const T& getElement(size_t row, size_t col) const{return (*this)(row,col);} //alias to operator(r,c) const;

		virtual T& operator()(const std::vector<size_t>& elementCoord) = 0; ///< set element at coordinate [x,y,z,...] specified by the vector elementCoord
		virtual const T& operator()(const std::vector<size_t>& elementCoord) const = 0; ///< get element at coordinate [x,y,z,...] specified by the vector elementCoord
		const T& getElement(const std::vector<size_t>& subs) const{return (*this)(subs);} //alias to operator() const;

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

		//! Concatenate with generic array
		//virtual ArrayBase& concatenate(const ArrayBase& b, size_t dim) = 0;

    };
}

template<typename T> void disp(const extras::ArrayBase<T>& A) {A.disp();}
template<typename T> size_t numel(const extras::ArrayBase<T>& obj){return obj.numel();}
template<typename T> bool isempty(const extras::ArrayBase<T>& obj){return obj.isempty();}
template<typename T> size_t nRows(const extras::ArrayBase<T>& obj){return obj.nRows();}
template<typename T> size_t nCols(const extras::ArrayBase<T>& obj){return obj.nCols();}
template<typename T> size_t ndims(const extras::ArrayBase<T>& obj){return obj.ndims();}
template<typename T> std::vector<size_t> dims(const extras::ArrayBase<T>& obj){ return obj.dims();}
