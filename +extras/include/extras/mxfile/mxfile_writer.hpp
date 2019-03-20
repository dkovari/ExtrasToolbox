/*----------------------------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari & James Merrill, Emory University
All rights reserved.
-----------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include <cstdio>
#include <mex.h>
//#include <zlib.h>
#include <list>
#include <string>
#include <vector>

namespace extras {namespace mxfile {
	// Data Structure
	struct SerialData {
		size_t nbytes; // number of bytes of data to write
		const mxArray* data;  //the mxArray to be written to disk
	};

	//Wrapper for fwrite; change as needed later
	size_t write(const void * data, size_t n_bytes, size_t count, FILE * stream) {
		return fwrite(data, n_bytes, count, stream);
	}

	/*/compressed version
	size_t write(const void * data, size_t n_bytes, size_t count, gzFile stream) {
		return gzwrite(stream, data, n_bytes * count);
	}*/

	//! Flattens list of mxArray*s into a serialized list
	//! Cells and struct are decomposed into a serialized list of numeric/char type arrays
	//! resulting serialized data is returned as a list of "SerialData" a simple struct containing a
	//! const mxArray* and nbytes, the size the struct will be on the disk (when uncompressed)
	std::list<SerialData> Serialize(size_t nrhs, const mxArray** prhs) {
		std::list<SerialData> out;
		for (size_t i = 0; i < nrhs; i++) {
			SerialData l;
			switch (mxGetClassID(prhs[i])) {
			case mxCELL_CLASS:
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + sizeof(size_t) * mxGetNumberOfDimensions(prhs[i]);
				//type, ndim, dims
				l.data = prhs[i];
				out.push_back(l);
				for (size_t j = 0; j < mxGetNumberOfElements(prhs[i]); ++j) {

					const mxArray * c = mxGetCell(prhs[i], j);

					out.splice(out.end(), Serialize(1, &c));

				}
				break;
			case mxSTRUCT_CLASS: {
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + mxGetNumberOfDimensions(prhs[i]) + sizeof(std::string);
				for (size_t j = 0; j < mxGetNumberOfFields(prhs[i]); ++j) {
					l.nbytes += sizeof(char) * std::string(mxGetFieldNameByNumber(prhs[i], j)).length();
				}
				//type, ndims, dims, nfields, field names
				l.data = prhs[i];
				out.push_back(l);
				for (size_t j = 0; j < mxGetNumberOfElements(prhs[i]); ++j) {
					for (size_t k = 0; k < mxGetNumberOfFields(prhs[i]); ++k) {
						const mxArray* c = mxGetFieldByNumber(prhs[i], j, k);
						out.splice(out.end(), Serialize(1, &c)); //unfold contents of fields
					}
				}
			}
				break;
			default:
				l.nbytes = sizeof(uint8_t) + sizeof(size_t) + sizeof(uint8_t) + sizeof(uint8_t); //type, ndims, isComplex, isInterleaved,
				l.nbytes += mxGetNumberOfDimensions(prhs[i]) * sizeof(size_t) + mxGetNumberOfElements(prhs[i]) * mxGetElementSize(prhs[i]) * (1 + mxIsComplex(prhs[i])); //  dims, data
				l.data = prhs[i];
				out.push_back(l);
			}
		}
		return out;
	}


	/** Helper class for file pointers
		* Provides a generic way to write to a file pointer
		* class can be derived and file-pointer and write method
		* can be redefined
	*/
	class FILE_WritePointer {
	protected:
		FILE* _fp = nullptr;
	public:
		virtual ~FILE_WritePointer() {};

		FILE_WritePointer(FILE* fp) :_fp(fp) {};

		//! write data to file pointer
		//! Input:
		//!		data: pointer to data to write
		//!		nbytes: number of bytes to write
		//! Return: number of bytes written
		virtual size_t write(const void* data, size_t nbytes) {
			return std::fwrite(data, 1, nbytes, _fp);
		}
	};

	/*/! ZLib Wrapper
	class ZFILE_WritePointer : public FILE_WritePointer {
	protected:
		gzFile _fp; //zlib file pointer
	public:
		ZFILE_WritePointer(gzFile fp) :FILE_WritePointer(nullptr), _fp(fp) {};
		virtual ~ZFILE_WritePointer() {};

		//! write data to file pointer
		//! Input:
		//!		data: pointer to data to write
		//!		nbytes: number of bytes to write
		//! Return: number of bytes written
		virtual size_t write(const void* data, size_t nbytes) {
			return gzwrite(_fp, data, nbytes);
		}
	};*/

	//! Loop over all arrays in the serialized list and write to FP
	void writeList(const std::list<SerialData>& dataList, FILE_WritePointer& FP) {
		for (auto& thisData : dataList) { //loop over all items in the list
			const mxArray* thisArray = thisData.data;
			uint8_t type = mxGetClassID(thisArray);
			size_t ndims = mxGetNumberOfDimensions(thisArray);

			size_t bytes_written = 0;

			//write type byte
			bytes_written += FP.write(&type, 1);

			//write ndim
			bytes_written += FP.write(&ndims, sizeof(size_t));

			//write dims
			bytes_written += FP.write((void*)mxGetDimensions(thisArray), sizeof(size_t)*ndims);

			switch (mxGetClassID(thisArray))
			{
			case mxCELL_CLASS:
				//nothing else to write
				break;
			case mxSTRUCT_CLASS:
			{
				//write number of field names
				size_t nfields = mxGetNumberOfFields(thisArray);
				bytes_written += FP.write(&nfields, sizeof(size_t));

				////////////
				// FOR EACH FIELD
				//write length of fieldname and fieldnames
				for (size_t n = 0; n < nfields; ++n) {
					std::string fieldname(mxGetFieldNameByNumber(thisArray, n));
					size_t len = fieldname.size();

					//write length of name
					bytes_written += FP.write(&len, sizeof(size_t));

					//write fieldname
					bytes_written += FP.write(fieldname.c_str(), len * sizeof(char));
				}
			}
				break;
			default: //all other types
			{
				//write complex
				uint8_t isComplex = (uint8_t)mxIsComplex(thisArray);
				bytes_written += FP.write(&isComplex, sizeof(uint8_t));

				//write interleaved flag
				uint8_t interFlag = 0; //default to not interleaved complex data
#if MX_HAS_INTERLEAVED_COMPLEX
				interFlag = 1;
#endif
				bytes_written += FP.write(&interFlag, sizeof(uint8_t));

				//Write data
				size_t numel = mxGetNumberOfElements(thisArray);
				size_t elsz = mxGetElementSize(thisArray);
				if (!isComplex) { //not complex, simple write
					bytes_written += FP.write(mxGetData(thisArray), numel*elsz);
				}
				else { //is complex
#if MX_HAS_INTERLEAVED_COMPLEX //interleaved data, size is 2x so regular copy works fine
					bytes_written += FP.write(mxGetData(thisArray), numel*elsz);
#else //not interleaved, need to write imag data explicitly
					bytes_written += FP.write(mxGetData(thisArray), numel*elsz);
					bytes_written += FP.write(mxGetImagData(thisArray), numel*elsz);
#endif
				}

			}
			}
		}
	}
}}