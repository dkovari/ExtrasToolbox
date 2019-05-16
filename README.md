# ExtrasToolbox
Useful functions and classes for MATLAB

## Including ExtrasToolbox in a project
You should install ExtrasToolbox using the Included .mltbx file or alternatively clone the repository and make sure the included .../+extras directory is listed on you MATLAB path.

## Building Included MEX Functions
If you install manually (without using the mltbx file), you will need to build the various MEX components. You can do that in two ways:

### Option 1: Using the included build script .../+extras/build_MexComponents.m
Simply run extras.build_MexComponents from the command line. It should build each of the required mex functions.

### Option 2: Using Visual Studio Solution
1) Define MATLAB_ROOT evironment variable
You must define MATLAB_ROOT in your system environmental variables.
It should point to the install directory of your version of MATLAB.
For example: 'C:\Program Files\MATLAB\R2018a' 

2) Open .../+extras/msvc/extras.sln
Retarget the projects if necessary.

3) Build StackWalker Project first. It is required by several other projects

4) Build all other projects as needed.


### Build Note
If building on Linux/Unix/Mac, make sure you have installed a working copy of zlib (a.k.a. libz) via your repository of choice.
Alternatively (and on Windows) you should be build zlib using the make files included in .../+extras/external_libs/zlib.

See .../+extras/external_libs/zlib/build_instructions.txt for details.

## Documentation
Documentation for the entire toolbox should be found in ExtrasToolbox/docs. However, you can also build the documentation using extras.docgenerator().

To generate documentation and store it to './ExtrasDocs/', run the following from the command prompt:
```matlab
>> extras.docgenerator('extras','ExtrasDocs');
```
The result will be a series of cross-referenced HTML pages which enumerate the available scripts, functions and classes.

You can also use MATLAB's built-in help system by calling 
```matlab
>> help extras.____
```

### C/C++ Header Documentation
To document the c++ headers used in the various MEX files, you can use doxygen (http://www.doxygen.nl/) a DoxyWizard file is located at:

.../+extras/include/Doxyfile
