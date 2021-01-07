/*
mexObjectInterfaceTest.cpp

This mex file creates an interface managing a c++ object using the mexObjectInterface class

/*--------------------------------------------------
Copyright 2021, Daniel T. Kovari
All rights reserved.
----------------------------------------------------*/

#include <extras/SessionManager/mexObjectInterface.hpp>

#include <string>
#include <ctime>
#include <sstream>

class TestObj {
private:
	std::string creation_time;
public:
	static std::string stat_str;
	std::string mystr;
	TestObj() {
		std::stringstream ss;
		std::time_t t = std::time(0); //current time;
		std::tm* now = std::localtime(&t);
		ss << (now->tm_year + 1900) << '-'
			<< (now->tm_mon + 1) << '-'
			<< now->tm_mday;
	}
	const std::string& creationTime() const { return creation_time; }

	static std::string testStatic() { return "test static function"; }
};
std::string TestObj::stat_str = "this is a test static variable";


class TestInterface : virtual public extras::SessionManager::mexObjectInterface<TestObj> {
	typedef std::shared_ptr<TestObj> pTestObj;
protected:
	void creationTime_method(pTestObj tObj, int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) {
		plhs[0] = extras::cmex::MxObject(tObj->creationTime());
	}
	void get_ct(pTestObj tObj, int nlhs, mxArray** plhs) {
		plhs[0] = extras::cmex::MxObject(tObj->creationTime());
	}
	void get_mystr(pTestObj tObj, int nlhs, mxArray** plhs) {
		plhs[0] = extras::cmex::MxObject(tObj->mystr);
	}
	void set_mystr(pTestObj tObj, int nlhs, mxArray** plhs,int nrhs, const mxArray** prhs) {
		if (nrhs < 1) {
			throw(std::runtime_error("nrhs<1, need input argument to set mystr"));
		}
		tObj->mystr = extras::cmex::getstring(prhs[0]);

	}
	void testStatic(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) {
		plhs[0] = extras::cmex::MxObject(TestObj::testStatic());
	}

	void get_stat_str(int nlhs, mxArray** plhs) {
		plhs[0] = extras::cmex::MxObject(TestObj::stat_str);
	}
public:
	TestInterface() {
		using namespace std::placeholders;
		addFunction("creationTime", std::bind(&TestInterface::creationTime_method, this, _1, _2, _3, _4, _5));
		addVariable("ct", std::bind(&TestInterface::get_ct, this, _1, _2, _3));
		addVariable("mystr", std::bind(&TestInterface::get_mystr, this, _1, _2, _3),
			std::bind(&TestInterface::set_mystr, this, _1, _2, _3, _4, _5));
		addStaticVariable("stat_str", std::bind(&TestInterface::get_stat_str, this, _1, _2));
		addStaticFunction("testStatic", std::bind(&TestInterface::testStatic, this, _1, _2, _3, _4));
	}
};


TestInterface g_testInterface;
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
	g_testInterface.mexFunction(nlhs, plhs, nrhs, prhs);
}