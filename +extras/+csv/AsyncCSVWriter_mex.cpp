/*
AsyncCSVWriter

This mex file creates an asynchronous processor object which writes char and numeric variables to a text file as csv

/*--------------------------------------------------
Copyright 2020, Daniel T. Kovari
All rights reserved.
----------------------------------------------------*/

#include <extras/fileio/AsyncCSVWriter.hpp>

extras::SessionManager::ObjectManager<extras::fileio::AsyncCSVWriter> manager;
extras::fileio::AsyncCSVWriterInterface<extras::fileio::AsyncCSVWriter, manager> ep2_interface; //create interface manager for the ExampleProcessor

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
	ep2_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}