#include <extras/async/AsyncProcessor.hpp>
#include <extras/mxfile/mxfile_writer.hpp>
#include <string>

class WriteProcessor : public extras::async::AsyncProcessor {
protected:
	/// method for Processing Tasks in the task list
	virtual extras::cmex::mxArrayGroup ProcessTask(const extras::cmex::mxArrayGroup& args) {
		extras::cmex::mxArrayGroup out = args;
		size_t nrhs = args.size;
		const mxArray** prhs = args.operator const mxArray **;		
		if (nrhs < 1) {
			throw(std::runtime_error("Requires at least one argument"));
		}

		std::string filepath = extras::cmex::getstring(prhs[0]);

		mexPrintf("Will use %s for writing\n", filepath.c_str());

		// serialize the data
		mexPrintf("About to serialize data\n");
		auto sD = extras::mxfile::Serialize(nrhs - 1, &(prhs[1])); //skip first argument;
		
		// write to file
		mexPrintf("Writing serialized data\n");
		extras::mxfile::writeList(sD, filepath.c_str());

		std::this_thread::sleep_for(std::chrono::milliseconds(500)); //let some time pass
		return out;
	}
};

	extras::SessionManager::ObjectManager<WriteProcessor> manager;
	extras::async::AsyncMexInterface<WriteProcessor, manager> wp_interface; //create interface manager for the WriteProcessor

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	wp_interface.mexFunction(nlhs, plhs, nrhs, prhs);
}
