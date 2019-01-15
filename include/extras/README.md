# Namespace: extras
A collection of functions and classes that make numeric computing in c++ much easier. This namespace also includes the nested namespace: extras::cmex, which contains functions/classes that make writing MEX code much easier.

## Contents
* ./cmex
  - Directory containing code included in the extras::cmex namespace
* ArrayBase.hpp
  * extras::ArrayBase<T>
    - Class interface definition for a generic multidimensional array.
* Array.hpp
  * extras::Array<T>
    - Implementation of ArrayBase<T> using c-native (i.e. malloc/free) memory management.
      * Note: Data is stored in column_major order.
