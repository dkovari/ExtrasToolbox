#include <cstddef>
#include <cstdio>
#include <mex.h>
#include <list>
#include <string>
#include <vector>

namespace extras {
	namespace mxfile {
		//Wrapper for fread; change as needed later (e.g., gzfread)
		size_t read(void * data, size_t size, size_t count, FILE * stream) {
			return fread(data, size, count, stream);
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