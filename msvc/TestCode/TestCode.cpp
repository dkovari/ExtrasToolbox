// TestCode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <tuple>
#include <map>
#include <string>


class A {
public:
	static bool sV;

	A() {
		std::cout << "sV: " << sV << std::endl;
		sV = false;
	}
};
bool A::sV = true;

template<typename T>
class B : public A {
public:
	T val = 0;
	B() {
		std::cout << "B::Type: " << typeid(val).name() << std::endl;
	}
};


int main()
{
	using namespace std;

	B<int> one;
	B<double> two;


    return 0;
}

