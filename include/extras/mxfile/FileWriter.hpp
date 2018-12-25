#include <cstddef>
#include <cstdio>
#include <mex.h>
#include <list>
#include <string>
#include <vector>

namespace extras {
	namespace mxfile {
		// Data Structure
		struct SerialData {
			size_t nbytes; // number of bytes of data to write
			const mxArray* data;  //the mxArray to be written to disk
		};

		//Wrapper for fwrite; change as needed later
		size_t write(const void * data, size_t n_bytes, size_t count, FILE * stream) {
			return fwrite(data, n_bytes, count, stream);
		}



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
		
		void writeList(const std::list<SerialData> dataList, const char* filepath) {
			//add in try-catch block for fopen failure, or otherwise)
			FILE* fp = fopen(filepath, "wb");
			for( auto& thisData : dataList) {
				auto thisArray = thisData.data;
				uint8_t type = mxGetClassID(thisArray);
				size_t ndims = mxGetNumberOfDimensions(thisArray);
				switch (mxGetClassID(thisArray)) {
				case mxCELL_CLASS:
					//write type byte
					write(&type, 1, 1, fp);

					//write ndim
					write(&ndims, sizeof(size_t), 1, fp);

					//write dims
					write((void*)mxGetDimensions(thisArray), sizeof(size_t), ndims, fp);

					break;
				case mxSTRUCT_CLASS: {

					//write type byte
					write(&type, 1, 1, fp);

					//write ndim
					write(&ndims, sizeof(size_t), 1, fp);

					//write dims
					write((void*)mxGetDimensions(thisArray), sizeof(size_t), ndims, fp);

					//write number of field names
					size_t nfields = mxGetNumberOfFields(thisArray);
					write(&nfields, sizeof(size_t), 1, fp);

					//write length of fieldname and fieldnames
					for (size_t n = 0; n < nfields; ++n) {
						std::string fieldname(mxGetFieldNameByNumber(thisArray, n));
						size_t len = fieldname.size();
						write((void*)&len, sizeof(size_t), 1, fp);
						write((void*)fieldname.c_str(), sizeof(char), fieldname.size(), fp);
					}
				}
					//write type
					//ndim
					//dim
					//nfields
					//for each field
					//length
					//field chars
					break;
				default: //all other types are numeric-ish
					// find the number from the enum in StructWriter
					mexPrintf("Before writes fp=%i\n", ftell(fp));
					//write type
					write(&type, 1, 1, fp);
					mexPrintf("after type fp=%i\n", ftell(fp));
					

					//write ndim
					write(&ndims, sizeof(size_t), 1, fp);
					mexPrintf("after ndims fp=%i\n", ftell(fp));

					//write dims
					write((void*)mxGetDimensions(thisArray), sizeof(size_t), ndims, fp);
					mexPrintf("after dims fp=%i\n", ftell(fp));

					//write complex
					uint8_t isComplex = (uint8_t)mxIsComplex(thisArray);
					write(&isComplex, 1, 1, fp);
					mexPrintf("after isComplex fp=%i\n", ftell(fp));

					//write interleaved flag
					uint8_t interFlag = 0; //default to not interleaved complex data
#if MX_HAS_INTERLEAVED_COMPLEX
					interFlag = 1;
#endif
					mexPrintf("\t About to write interFlag:%d\n", interFlag);
					write(&interFlag, 1, 1, fp);
					mexPrintf("after interFlag fp=%i\n", ftell(fp));

					//Write data
					if(!isComplex){
						//write all data at once
						/*mexPrintf("\t About to non-complex: ");
						for (size_t b = 0; b < 8; b++) {
							mexPrintf("%02x", ((uint8_t*)mxGetData(thisArray))[b]);
						}
						mexPrintf("\n");
						uint64_t val = ((uint64_t*)mxGetData(thisArray))[0];
						mexPrintf("\t            should be: %016x\n", val);
						mexPrintf("\t        from uint64_t: %f\n", (double)val);*/

						write(mxGetData(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
					}else{
#if MX_HAS_INTERLEAVED_COMPLEX
						write(mxGetData(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
#else
						//write real data first
						write(mxGetPr(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
						//write complex data second
						write(mxGetPi(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
#endif
					}
					break;

				}
			}
			fclose(fp);
			return;
		}


		//Wrapper for fread; change as needed later
		size_t read(void * data, size_t size, size_t count, FILE * stream) {
			return fread(data, size, count, stream);
		}


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

				//I don't trust this code. I think there's an issue with null termination, and the const keyword
				//read fieldnames + lengths'

				char** fieldnames = new char*[nfields]; //initialize string array of field names
				size_t len;
				for (size_t n = 0; n < nfields; ++n) {
					read(&len, sizeof(size_t), 1, fp); //read field name length
					fieldnames[n] = new char[len];
					read((void *)fieldnames[n], sizeof(char), len, fp); //read field name
					fieldnames[n][len] = '\0';
				}

				out = mxCreateStructArray(ndim, dims, nfields, (const char**) fieldnames);

				/*for (size_t n = 0; n < nfields; ++n) {
					delete[] fieldnames[n];
				}
				delete[] fieldnames;
				*/
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
		
		std::vector<mxArray*> readFile(const char* filename)
		{
			FILE* fp = fopen(filename, "rb");

			std::vector<mxArray*> out;

			while (!feof(fp)) { //read until end of file
				try { out.push_back(readNext(fp)); }
				catch(std::exception e){}
			}
			fclose(fp);
			return out;
		}
		
		//read one line and return
		mxArray* readline(const char* filename) {
			FILE* fp = fopen(filename, "rb");

			mxArray* thisArray = extras::mxfile::readNext(fp);

			/*while (!feof(fp)) { //read until end of file
			}
			*/
			fclose(fp);
			return thisArray;
		}
	}
}