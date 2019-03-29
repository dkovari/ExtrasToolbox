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



class C {
public:
	double val = 1;
	C() {
		std::cout << "construct C, val: " << val << std::endl;
	}
};


class D:protected C{
public:
	D() {
	std::cout << "construct D" << std::endl;
	}
};

void myFunction(const C& c) {
	std::cout << "myFunction c.val" << c.val<<std::endl;
}

int main()
{
	using namespace std;

	D d;
	myFunction(d);


    return 0;
}

