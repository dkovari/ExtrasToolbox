#include "stdafx.h"

#include <extras/stacktrace_error.hpp>

void Func5() { 
	throw(extras::stacktrace_error("Func5 throw!!!"));

}
void Func4() { Func5(); }
void Func3() { Func4(); }
void Func2() { Func3(); }
void Func1() { Func2(); }

int main()
{
	try{
		Func1();
	}
	catch (std::exception& e) {
		//EXCEPTION_POINTERS* pExp = GetExceptionInformation();

		printf(e.what());
	}
	

	system("pause");
	return 0;
}