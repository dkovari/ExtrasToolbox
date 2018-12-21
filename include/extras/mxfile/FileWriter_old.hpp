#include <cstddef>
#include <cstdio>
#include <mex.h>
#include <list>
#include <string>
#include <matrix.h>

namespace extras {
	namespace mxfile {
		// Data Structure
		struct SerialData {
			size_t nbytes; // number of bytes of data to write
			const mxArray* data;
		};




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

						mxArray* c = mxGetCell(prhs[i], j);
						out.splice(Serialize(1, &c )); //unfold contents of cells
					}
					break;
				case mxSTRUCT_CLASS:
					l.nbytes = sizeof(uint8_t) + sizeof(size_t) + mxGetNumberOfDimensions(prhs[i]) + sizeof(std::string);
					for (size_t j = 0; j < mxGetNumberOfFields(prhs[i]); j++) {
						l.nbytes += sizeof(std::string) * std::string(mxGetFieldNameByNumber[j]).length();
					}
					//type, ndims, dims, nfields, field names
					l.data = prhs[i];
					out.push_back(l);
					for (size_t j = 0; j < mxGetNumberOfElements(prhs[i]); ++j) {
						for (size_t k = 0; k < mxGetNumberofFields(prhs[i]); ++k) {
							out.splice(Serialize(1, mxGetFieldByNumber(prhs[i], j, k))); //unfold contents of fields
						}
					}
					break;
				default:
					l.nbytes = sizeof(uint8_t) + sizeof(size_t) + sizeof(uint8_t) + sizeof(uint8_t) + mxGetNumberOfDimensions(prhs[i]) * sizeof(size_t) + mxGetNumberOfElements(prhs[i]) * mx.GetElementSize(prhs[i]) * (1 + mxIsComplex(prhs[i])); // type, ndims, isComplex, isInterleaved, dims, data
					l.data = prhs[i];
					out.push_back(l);
				}
				return out;
			}
		}

		void writeList(const std::List<SerialData> dataList, std::string filepath) {
			//add in try-catch block for fopen failure, or otherwise)
			FILE* fp = fopen(filepath, "wb");
			for (size_t i = 0; i < dataList.size(); i++) {
				mxArray* thisArray = dataList[i].data;
				switch (mxGetClassID(thisArray)) {
				case mxCELL_CLASS:
					//write type byte
					uint8_t type = ;
					write(&type, 1, 1, fp);

					//write ndim
					size_t ndims = mxGetNumberDimensions(thisArray);
					write(&ndims, sizeof(size_t), 1, fp);

					//write dims
					write((void*)mxGetdimensions(thisArray), sizeof(size_t), ndims, fp);

					break;
				case mxSTRUCT_CLASS:

					//write type byte
					uint8_t type = ;
					write(&type, 1, 1, fp);

					//write ndim
					size_t ndims = mxGetNumberDimensions(thisArray);
					write(&ndims, sizeof(size_t), 1, fp);

					//write dims
					write((void*)mxGetdimensions(thisArray), sizeof(size_t), ndims, fp);

					//write number of field names
					size_t nfields = mxGetNumberofFields(thisArray);
					write(&nfields, sizeof(size_t), 1, fp);

					//write length of fieldname and fieldnames
					for (size_t n = 0; n < nfields; ++n) {
						std::string fieldname(mxGetFieldNameByNumber(thisArray, n))
							size_t len = fieldname.size();
						write((void*)&len, sizeof(size_t), 1, fp);
						write((void*)fieldname.c_str(), sizeof(char), fieldname.size(), fp);
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

					//write type
					uint8_t type = ;
					write(&type, 1, 1, fp);

					//write ndim
					size_t ndims = mxGetNumberDimensions(thisArray);
					write(&ndims, sizeof(size_t), 1, fp);

					//write dims
					write((void*)mxGetdimensions(thisArray), sizeof(size_t), ndims, fp);

					//write complex
					uint8_t isComplex = (uint8_t)mxIsComplex(thisArray);
					write(&isComplex, 1, 1, fp);

					//write interleaved flag
					uint8_t interFlag = 0; //default to not interleaved complex data
#if MX_HAS_INTERLEAVED_COMPLEX
					interFlag = 1;
#endif
					write(&interFlag, 1, 1, fp);

					//Write data
					if (isComplex&&~interFlag) {
						//write real data first
						write(mxGetPr(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
						//write complex data second
						write(mxGetPi(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
					}
					else {
						//write all data at once
						write(mxGetData(thisArray), mxGetElementSize(thisArray), mxGetNumberOfElements(thisArray), fp);
					}
					break;

				}
			}
			fclose(fp);
			return;
		}

		//Wrapper for fwrite; change as needed later
		size_t write(const void * data, size_t n_bytes, size_t count, FILE * stream) {
			fwrite(data, n_bytes, count, stream);
		}

		//Wrapper for fread; change as needed later
		size_t read(const void * data, size_t size, size_t count, FILE * stream) {
			fread(data, size, count, stream);
		}


		mxArray* readNext(FILE* fp) {
			mxArray* out = nullptr;

			// read type
			mxClassEnum thisType = ; //helper function should convert a uint8_t to mxClassEnum
			//read ndim
			size_t ndim;
			read(&ndim, sizeof(size_t), 1, fp);
			//read dim
			size_t* dims = new size_t[ndim];
			//for each of ndim, read dim and set
			for (size_t i; i < ndim; ++i) {
				read(&dims[i], sizeof(size_t), 1, fp);
			}

			//create appropriate type
			switch (thisType) {
			case mxCELL_CLASS:
				out = mxCreateCellArray(ndim, dims);
				break;
			case mxSTRUCT_CLASS:
				//read number of fields
				size_t nfields;
				read(&nfields, sizeof(size_t), 1, fp);

				//initialize pointers for field name lengths & field names
				size_t* namelengths = new size_t[nfields];
				std::string* fieldnames = new std::string[nfields];

				//for each element, for each field, read field names and set;
				for (size_t i = 0; i < nfields; ++i) {
					read(&namelengths[i], sizeof(size_t), 1, fp); //read field name length
					read(&fieldnames[i], namelengths[i] * sizeof(size_t), 1, fp); //read fieldname
				}
				out = mxCreateStructArray(ndim, dims, nfields, fieldnames);
				break;
			default: // need more cases here; otherwise we assume all numeric arrays are doubles
				uint8_t complexity;
				read(&complexity, sizeof(uint8_t), 1, fp);
				uint8_t interFlag;
				read(&interFlag, sizeof(uint8_t), 1, fp);
				if (interflag) {
					out = mxCreateNumericArray(ndim, dims, type, complexity);
					read(out, sizeof(double), mxGetNumberOfElements(out), fp);
				}
				else {
					out = mxCreateNumericArray(ndim, dims, type, complexity)
						double* pr = new double[mxGetNumberOfElements(out)];
					double* pi = new double[mxGetNumberOfElements(out)];
					read(pr, sizeof(double), mxGetNumberOfElements / 2, fp);
					read(pi, sizeof(double), mxGetNumberOfElements / 2, fp);
					mxSetPr(out, pr);
					mxSetPi(out, pi);


				}
				//read complexity
				//read interleaved flag
				//read the data
				//set out
				return out;
			}


			std::vector<mxArray*> ReadFile(const char* filename) {
				FILE* fp = fopen(filepath, "rb");

				std::vector<mxArray*> out;

				while (~eof(fp)) { //read until end of file
					thisArray = readNext(fp);
					switch (mxGetClassID(thisArray)) {
					case mxCELL_CLASS:
						//given dims of cell array, fill with the appropriate number of numeric arrays, push to out
						break;
					case mxSTRUCT_CLASS:
						//populate the dims, fields
						break;
					default:
					}
					//read first byte, if read successfully,

					//increase out size by one
					out.

				}
				fclose(fp);
			}
		}
	}