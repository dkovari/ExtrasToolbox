// TestCode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <tuple>


template<typename T=double>
struct A {
	int one;
	T two;

	A(int a, T b) :one(a), two(b) {};
};
struct B: public A<> {

	B(int x, int y) :A(x,y) {};
};

int main()
{
	using namespace std;
	B myB(1, 2);
	cout << "B.one: " << myB.one << endl;
	cout << "B.two: " << myB.two << endl;
	cout << "type:" << typeid(myB.two).name() << endl;
    return 0;
}

