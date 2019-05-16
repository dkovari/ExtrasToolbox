# Namespace: extras::cmex
All of the header in this directory define classes and functions in the extras::cmex namespace. The purpose of this namespace is to contain functions/classes that rely upon MATLAB's "C MEX API."  NOTE: Even if these functions are written in c++ they do not use the 'new' "C++ MEX API." Therefore you cannot use the C++ Data API, Engine API for C++, nor the C++ MEX Applications API.

## Contents
* mexextras.hpp
  * makes writing mex-code easier
* mxobject.hpp
  * extras::cmex::MxObject
    - Wrapper for mxArray*'s, automatically manages data allocation and can be used to create/manage data persistence beyond each MEX function call.
  * MxObjectException - Exceptions thrown by MxObjects
* NumericArray.hpp
  * extras::cmex::NumericArray
    - Wrapper for numeric-type mxArray*'s implementing the extras:ArrayBase interface. NumericArrays also inherit from MxObject
* type2ClassID.hpp
  * mxClassID type2ClassID<type>()
    - Template function that returns the mxClassID corresponding to a standard c type
  * bool sametype<type>(mxClassID cID)
    - returns true if type corresponds to type required by mxClassID
  * bool sametype<type>(const mxArray* mA)
    - returns true if type corresponds to type required by the mxArray*
* mxArrayGroup.hpp
  * extras::cmex::mxArrayGroup()
    * class that manages arrays of mxArray*
    * each pointer in the array of pointers is converted to persistent so that it can be accessed after a mexFunction call ends.
