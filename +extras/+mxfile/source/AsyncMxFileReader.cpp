/*----------------------------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari & James Merrill, Emory University
All rights reserved.
-----------------------------------------------------------------------*/

/** COMPRESSION INCLUDES & LIBS          ****************************
=====================================================================
This header depends on ZLIB for reading and writing files compressed
with the gz format. Therefore you need to have zlib built/installed
on your system.

A version of ZLIB is included with the ExtrasToolbox located in
.../+extras/external_libs/zlib
Look at that folder for build instructions.
Alternatively, is you are using a *nix-type system, you might have
better luck using your package manager to install zlib.

When building, be sure to include the location of zlib.h and to link to
the compiled zlib-lib files.
************************************************************************/

#include <extras/mxfile/mxfile_reader.hpp>

extras::SessionManager::ObjectManager<extras::mxfile::AsyncMxFileReader> manager;
extras::mxfile::AsyncMxFileReaderInterface<extras::mxfile::AsyncMxFileReader, manager> mex_interface; //create interface manager for the MxFileReader

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	mex_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}