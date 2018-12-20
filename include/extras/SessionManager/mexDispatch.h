#pragma once
/*
Derived from MEX Dispatch Library by Kota Yamaguchi

Use this header to define easy-to-use macros for creating multi-operation mex interfaces

Resulting mex files will have the syntax
> [varargour] = yourMexFcn('FunctionName',arg1,arg2,...)

Couple this with ObjectManager.h to create an interface to C++ objects that live
past the end of a mex call
*/

#include <mex.h>
#include <map>
#include <memory>
#include <string>
#include <exception>

namespace mexdispatch {

	typedef bool OperationNameAdmitter(const std::string& name);

	class OperationCreator;
	inline void CreateOperation(OperationNameAdmitter* admitter,
		OperationCreator* creator);

	/** Abstract operation class. Child class must implement operator().
	*/
	class Operation {
	public:
		/** Destructor.
		*/
		virtual ~Operation() {}
		/** Execute the operation.
		*/
		virtual void operator()(int nlhs,
			mxArray *plhs[],
			int nrhs,
			const mxArray *prhs[]) = 0;
	};

	/** Base class for operation creators.
	*/
	class OperationCreator {
	public:
		/** Register an operation in the constructor.
		*/
		explicit OperationCreator(OperationNameAdmitter* admitter) {
			CreateOperation(admitter, this);
		}
		/** Destructor.
		*/
		virtual ~OperationCreator() {}
		/** Implementation must return a new instance of the operation.
		*/
		virtual Operation* create() = 0;
	};

	/** Implementation of the operation creator to be used as composition in an
	* Operator class.
	*/
	template <class OperationClass>
	class OperationCreatorImpl : public OperationCreator {
	public:
		explicit OperationCreatorImpl(OperationNameAdmitter* admitter,
			const char* tag) :
			OperationCreator(admitter) {
			if (tag)
				mexPrintf("Tag: %s\n", tag);
		}
		virtual Operation* create() { return new OperationClass; }
	};

	/** Factory class for operations.
	*/
	class OperationFactory {
	public:
		typedef std::map<OperationNameAdmitter*, OperationCreator*> RegistryMap;

		/** Register a new creator.
		*/
		friend void CreateOperation(OperationNameAdmitter* admitter,
			OperationCreator* creator);
		/** Create a new instance of the registered operation.
		*/
		static Operation* create(const std::string& name) {
			RegistryMap::const_iterator it = find(name);
			return (it == registry()->end()) ?
				static_cast<Operation*>(NULL) : it->second->create();
		}
		/** Obtain a pointer to the registration table.
		*/
		static RegistryMap* registry() {
			static RegistryMap registry_table;
			return &registry_table;
		}

	private:
		static RegistryMap::const_iterator find(const std::string& name) {
			RegistryMap::const_iterator it;
			for (it = registry()->begin(); it != registry()->end(); it++) {
				if ((*it->first)(name))
					return it;
			}
			return it;
		}
	};

	/** Register a new creator in OperationFactory.
	*/
	inline void CreateOperation(OperationNameAdmitter* admitter,
		OperationCreator* creator) {
		OperationFactory::registry()->insert(std::make_pair(admitter, creator));
	}



	void handle_function_exception(std::exception_ptr eptr,const char* operation_name)
	{
		try {
			if (eptr) {
				std::rethrow_exception(eptr);
			}
		}
		catch (const std::exception& e) {
			std::string source("mexdispatch:MEX_DEFINE:");
			source += operation_name;
			mexErrMsgIdAndTxt(source.c_str(),"Caught exception: %s", e.what());
		}
	}
} //end mexdispatch namespace


/** Define a MEX API function. Example:
*
* MEX_DEFINE(myfunc) (int nlhs, mxArray *plhs[],
*                     int nrhs, const mxArray *prhs[]) {
*   if (nrhs != 1 || nlhs > 1)
*     mexErrMsgTxt("Wrong number of arguments.");
*   ...
* }
*/
#define MEX_DEFINE(name) \
class Operation_##name : public mexdispatch::Operation { \
 public: \
  virtual void operator()(int nlhs, \
                          mxArray *plhs[], \
                          int nrhs, \
                          const mxArray *prhs[]); \
 private: \
  static bool Operation_Admitter(const std::string& func) { \
    return func == #name;\
  } \
  static const mexdispatch::OperationCreatorImpl<Operation_##name> creator_; \
}; \
const mexdispatch::OperationCreatorImpl<Operation_##name> \
    Operation_##name::creator_(Operation_##name::Operation_Admitter, NULL); \
void Operation_##name::operator()


/*
Define mexFunction() for MATLAB interface
*/
#define MEX_DISPATCH \
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {\
	std::exception_ptr eptr;\
	if (nrhs < 1 || !mxIsChar(prhs[0]))\
		mexErrMsgIdAndTxt("mexdispatch:argumentError", "Invalid argument: missing operation.");\
	char* tmp = mxArrayToString(prhs[0]);\
	std::string operation_name(tmp);\
	mxFree(tmp);\
	std::unique_ptr<mexdispatch::Operation> operation(mexdispatch::OperationFactory::create(operation_name));\
	if (operation.get() == NULL) {\
		mexErrMsgIdAndTxt("mexdispatch:argumentError",\
			"Invalid operation: %s", operation_name.c_str());\
	}\
	try {\
		(*operation)(nlhs, plhs, nrhs - 1, prhs + 1);\
	}\
	catch (...) {\
		eptr = std::current_exception();\
	}\
	mexdispatch::handle_function_exception(eptr, operation_name.c_str());\
}
