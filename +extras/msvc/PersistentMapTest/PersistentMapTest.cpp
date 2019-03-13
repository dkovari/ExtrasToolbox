/*--------------------------------------------------
Copyright 2018-2019, Daniel T. Kovari, Emory University
All rights reserved.
----------------------------------------------------*/
#include <extras/cmex/PersistentMxMap.hpp>
#include <list>

// ParameterMxMap with default values
struct MyMap: public extras::cmex::ParameterMxMap {
	double special_val = 1;
	MyMap() {
		_map["MyMap1"] = mxCreateDoubleScalar(1);
		_map["MyMap2"] = mxCreateDoubleScalar(1);
	}

	virtual ~MyMap() {};
};


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	using namespace extras::cmex;

	mexPrintf("Create ParameterMxMap map1\n");
	mexEvalString("pause(0.2)");

	ParameterMxMap map1;

	mexPrintf("set map1 from parameters\n");
	mexEvalString("pause(0.2)");

	map1.setParameters(nrhs, prhs);
	
	mexPrintf("set plhs[0] to map1 via cell\n");
	mexEvalString("pause(0.2)");

	map1["dan123"] = mxCreateDoubleScalar(123);

	plhs[0] = map1.map2cell();

	mexPrintf("Create shared ptr to ParameterMxMap\n");
	mexEvalString("pause(0.2)");

	std::shared_ptr<ParameterMxMap> pMap = std::make_shared<ParameterMxMap>();

	mexPrintf("Map Ptr size: %d\n", pMap->size());
	mexEvalString("pause(0.2)");

	mexPrintf("set ptr from parameters\n");
	mexEvalString("pause(0.2)");
	
	pMap->setParameters(nrhs, prhs);
	mexPrintf("Map Ptr size: %d\n", pMap->size());
	mexEvalString("pause(0.2)");

	(*pMap)["dan_test1"] = mxCreateDoubleScalar(100);

	if (nlhs > 1) {
		plhs[1] = pMap->map2cell();
	}

	mexPrintf("dan_test1: %f\n", mxGetScalar((*pMap)["dan_test1"]));


	//////////
	// list pair

	std::list< std::pair< std::string, std::shared_ptr<ParameterMxMap> > > mapList;

	mapList.push_front(std::pair< std::string, std::shared_ptr<ParameterMxMap> >(std::string("test1"), pMap));

	mexPrintf("List Size: %d\n", mapList.size());
	mexPrintf("Front string: %s\n", mapList.front().first.c_str());

	/////////////////
	// MyMap

	std::shared_ptr<ParameterMxMap> pMapBase = std::make_shared<MyMap>();

	mexPrintf("pMapBase['MyMap1']: %f\n", mxGetScalar((*pMapBase)["MyMap1"]));

	auto downCastPtr = std::dynamic_pointer_cast<MyMap>(pMapBase);
	mexPrintf("downCastPtr['MyMap1']: %f\n", mxGetScalar((*downCastPtr)["MyMap1"]));
	mexPrintf("specialVal: %f\n", downCastPtr->special_val);
	downCastPtr->special_val = 2;
	mexPrintf("specialVal: %f\n", downCastPtr->special_val);
	auto upPtr = std::dynamic_pointer_cast<ParameterMxMap>(downCastPtr);
	auto down2 = std::dynamic_pointer_cast<MyMap>(upPtr);
	mexPrintf("down2->specialVal: %f\n", down2->special_val);
}