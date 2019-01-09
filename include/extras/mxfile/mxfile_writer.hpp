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
	}
}