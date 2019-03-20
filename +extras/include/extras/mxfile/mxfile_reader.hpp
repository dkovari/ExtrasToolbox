#include <cstddef>
#include <cstdio>
#include <mex.h>
#include <list>
#include <string>
#include <vector>

#include <extras/cmex/mxobject.hpp>

namespace extras {
	namespace mxfile {
		


		class FILE_ReadPointer {
		protected:
			FILE* _fp = nullptr;
		public:
			FILE_ReadPointer(FILE* fp) :_fp(fp) {};

			//! attempts to read data from fp into dst
			//! returns number of bytes read
			size_t read(void* dst, size_t nbytes) {
				return std::fread(dst, 1, nbytes, _fp);
			}

			//! return true if eof has been reached
			bool eof() {
				return std::feof(_fp);
			}

			const char* error_msg() {
				return std::strerror(std::ferror(_fp));
			}
		};

		//! Read next mxArray from the file
		extras::cmex::MxObject readNext(FILE_ReadPointer& FP) {
			extras::cmex::MxObject out;

			// read type
			mxClassID thisType;
			uint8_t tmp;
			if (FP.read(&tmp, 1) < 1) {
				if (FP.eof()) {
					return out;//return empty array;
				}
				else {
					throw(FP.error_msg());
				}
			}

			//read ndim
			size_t ndim;
			if (FP.read(&ndim, sizeof(size_t)) < sizeof(size_t)) {
				if (FP.eof()) {
					throw("reached end of file while reading ndim");
				}
				else {
					throw(FP.error_msg());
				}
			}

			//read dims
			std::vector<size_t> dims(ndim);
			if (FP.read(dims.data(), sizeof(size_t)*ndim) < sizeof(size_t)*ndim) {
				if (FP.eof()) {
					throw("reached end of file while reading dims");
				}
				else {
					throw(FP.error_msg());
				}
			}

			//create array to hold data
			switch (thisType) {
			case mxCELL_CLASS:
			{
				out = mxCreateCellArray(ndim, dims.data());
				for (size_t n = 0; n < out.numel(); n++) {
					mxSetCell(out.getmxarray(), n, readNext(FP));
				}
			}
				break;
			case mxSTRUCT_CLASS:
			{
				//get nfields
				size_t nfields;
				if (FP.read(&nfields, sizeof(size_t)) < sizeof(size_t)) {
					if (FP.eof()) {
						throw("reached end of file while reading nfields");
					}
					else {
						throw(FP.error_msg());
					}
				}

				std::vector<std::vector<char>> fieldnames(nfields); //vector of strings in which names are stored
				std::vector<const char*> fnames(nfields); //vector of const char* used to call mxCreateStruct...
				for (size_t f = 0; f < nfields; f++) { //loop and read names
					size_t len;
					if (FP.read(&len, sizeof(size_t)) < sizeof(size_t)) {
						if (FP.eof()) {
							throw("reached end of file while reading length of fieldname");
						}
						else {
							throw(FP.error_msg());
						}
					}

					fieldnames[f].resize(len);
					if (FP.read(fieldnames[f].data(), len) < len) {
						if (FP.eof()) {
							throw("reached end of file while reading lfieldname");
						}
						else {
							throw(FP.error_msg());
						}
					}
					fnames[f] = fieldnames[f].data();
				}

				out = mxCreateStructArray(ndim, dims.data(), nfields, fnames.data());

				/// Loop and read fields
				for (size_t j = 0; j < out.numel(); ++j) {
					for (size_t k = 0; k < nfields; ++k) {
						mxSetFieldByNumber(out.getmxarray(), j, k, readNext(FP));
					}
				}

			}
				break;
			default:
			{
				//read complexity
				uint8_t isComplex;
				if (FP.read(&isComplex, 1) < 1) {
					if (FP.eof()) {
						throw("reached end of file while reading isComplex");
					}
					else {
						throw(FP.error_msg());
					}
				}

				//read interleaved
				uint8_t interFlag;
				if (FP.read(&interFlag, 1) < 1) {
					if (FP.eof()) {
						throw("reached end of file while reading interFlag");
					}
					else {
						throw(FP.error_msg());
					}
				}

				if (thisType == mxCHAR_CLASS) { //handle char
					out = mxCreateCharArray(ndim, dims.data());
					size_t elsz = out.elementBytes();
					if (FP.read(mxGetData(out.getmxarray()), elsz*out.numel()) < elsz*out.numel()) {
						if (FP.eof()) {
							throw("reached end of file while reading char data");
						}
						else {
							throw(FP.error_msg());
						}
					}
				}
				else if (!isComplex) { //real
					out = mxCreateNumericArray(ndim, dims.data(), thisType, mxREAL);
					size_t elsz = out.elementBytes();
					if (FP.read(mxGetData(out.getmxarray()), elsz*out.numel()) < elsz*out.numel()) {
						if (FP.eof()) {
							throw("reached end of file while reading real numeric data");
						}
						else {
							throw(FP.error_msg());
						}
					}
				}
				else { //complex
					if (interFlag) { //data is interleaved
					}
					else { //data isnot interleaved
					}
				}
			}
			}

		}



		//Read the next data structure from the given file pointer.
		mxArray* readNext(FILE* fp) {
			mxArray* out = nullptr;

			// read type
			mxClassID thisType;
			uint8_t tmp;
			if (read(&tmp, 1, 1, fp) == 0) {
				return nullptr;
			}
			thisType = (mxClassID)tmp;

			//read ndim
			size_t ndim;
			read(&ndim, sizeof(size_t), 1, fp);

			//for each dim, read and set
			size_t* dims = new size_t[ndim];
			for (size_t i = 0; i < ndim; ++i) {
				read(&dims[i], sizeof(size_t), 1, fp);
			}

			switch (thisType) {
			case mxCELL_CLASS:
				out = mxCreateCellArray(ndim, dims);

				for (size_t n = 0; n < mxGetNumberOfElements(out); ++n) {
					mxSetCell(out, n, readNext(fp));
				}
				break;
			case mxSTRUCT_CLASS: {
				//read number of fields
				size_t nfields;
				read(&nfields, sizeof(size_t), 1, fp);


				char** fieldnames = new char*[nfields]; //initialize string array of field names
				size_t len;
				for (size_t n = 0; n < nfields; ++n) {
					read(&len, sizeof(size_t), 1, fp); //read field name length
					fieldnames[n] = new char[len];
					read((void *)fieldnames[n], sizeof(char), len, fp); //read field name
					fieldnames[n][len] = '\0';
				}

				out = mxCreateStructArray(ndim, dims, nfields, (const char**)fieldnames);

				for (size_t n = 0; n < mxGetNumberOfElements(out); ++n) {
					for (size_t m = 0; m < mxGetNumberOfFields(out); ++m) {
						mxSetFieldByNumber(out, n, m, readNext(fp));
					}
				}
				break;
			}
			default: //Default case: Numeric arrays
				uint8_t complexity;
				read(&complexity, sizeof(uint8_t), 1, fp);
				uint8_t interFlag;
				read(&interFlag, sizeof(uint8_t), 1, fp);

				if (complexity) {
					out = mxCreateNumericArray(ndim, dims, thisType, mxCOMPLEX);
					if (interFlag) {
#if MX_HAS_INTERLEAVED_COMPLEX //this version of matlab is interleaved, and data is also interleaved
						read(mxGetData(out), mxGetElementSize(out), mxGetNumberOfElements(out), fp);
#else //this matlab is not interleaved, but data is interleaved
						uint8_t* rdata = (uint8_t*)mxGetData(out);
						uint8_t* idata = (uint8_t*)mxGetImagData(out);

						//loading interleaved data into separate arrays
						for (size_t n = 0; n < mxGetNumberOfElements(out); ++n) {
							read(&(rdata[n*mxGetElementSize(out)]), mxGetElementSize(out), 1, fp);
							read(&(idata[n*mxGetElementSize(out)]), mxGetElementSize(out), 1, fp);
						}

#endif
					}
					else {
#if MX_HAS_INTERLEAVED_COMPLEX //this version of matlab is interleaved, but data is not
						uint8_t* rdata = new uint8_t[mxGetNumberOfElements(out)*mxGetElementSize(out)];
						uint8_t* idata = new uint8_t[mxGetNumberOfElements(out)*mxGetElementSize(out)];

						read(rdata, mxGetElementSize(out) / 2, mxGetNumberOfElements(out), fp);
						read(idata, mxGetElementSize(out) / 2, mxGetNumberOfElements(out), fp);

						uint8_t* out_data = (uint8_t*)mxGetData(out);
						for (size_t n = 0; n < mxGetNumberOfElements(out); ++n) {

							memcpy(&(out_data[n*mxGetElementSize(out)]), &(rdata[n*mxGetElementSize(out) / 2]), mxGetElementSize(out) / 2);
							memcpy(&(out_data[n*mxGetElementSize(out) + mxGetElementSize(out) / 2]), &(idata[n*mxGetElementSize(out) / 2]), mxGetElementSize(out) / 2);
						}
						delete[] rdata;
						delete[] idata;

#else //this matlab is not interleaved, and data was not interleaved
						void* rdata = mxGetData(out);
						void* idata = mxGetImagData(out);
						read(rdata, mxGetElementSize(out), mxGetNumberOfElements(out), fp);
						read(idata, mxGetElementSize(out), mxGetNumberOfElements(out), fp);
#endif
					}
				}
				else {
					out = mxCreateNumericArray(ndim, dims, thisType, mxREAL);
					read(mxGetData(out), mxGetElementSize(out), mxGetNumberOfElements(out), fp);
				}

			}
			return out;
		}
	}
}