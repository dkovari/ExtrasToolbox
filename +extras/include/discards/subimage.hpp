#pragma once
#include <cstddef>


namespace extras{
    template<typename T,bool rowmajor=false>
    class subimage{
    protected:
        T* _data=nullptr;
        size_t _stride=0;
        size_t _r0=0;
        size_t _c0=0;

    public:

        // Create subimage within data array
        // dat: pointer to the data
        // dat_stride: stride of the original array
        // startRow: new starting row
        // startCol: new starting column
        // newHeight: number of rows in subimage
        // newWidth: number of cols in subimage
        // rowmajor(=false): flag if data is row major (i.e. data index corresponds to moving along a row, then column)
        //          subimage defaults to column major (data index moves along column, then row)
        subimage(T* dat, size_t dat_stride, size_t startRow, size_t startCol):
            _data(dat),
            _stride(dat_stride),
            _r0(startRow),
            _c0(startCol){};

        // returns index in original array using subimage row and col
        inline size_t getIndex(size_t r, size_t c){
            size_t index=0;
            if(rowmajor){
                index = _r0+r + (_c0+c)*_stride;
            }else{
                index = _c0+c + (_r0+r)*_stride;
            }

        }

        inline T& operator()(size_t r, size_t c){
            return _data[getIndex(r,c)];
        }
        inline const T& operator()(size_t r, size_t c) const{
            return _data[getIndex(r,c)];
        }

        size_t stride() const{return _stride;}
        T* data() const{return &(_data[getIndex(0,0)]);}

    };
}
