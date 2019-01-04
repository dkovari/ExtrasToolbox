// TestCode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <tuple>
#include <map>
#include <string>


template<typename T=double>
struct A {
	int one;
	T two;

	A(int a, T b) :one(a), two(b) {};
};
struct B: public A<> {

	B(int x, int y) :A(x,y) {};
};


template<typename T=double>
struct C {
	virtual void sz() const = 0;
};
template<> void C<double>::sz() const {
	using namespace std;
	cout << "C<double>::sz()" << endl;
}

struct D :public C<int> {
	void sz() const {
		using namespace std;
		cout << "D::sz()->C<int>" << endl;
	}
};


struct D2 :public C<> {
	void sz() const {
		C<>::sz();
	}
};


typedef std::map<std::string, int> MapT;
int main()
{
	using namespace std;

	MapT myMap;
	myMap.insert(MapT::value_type("A",1));
	myMap.insert(MapT::value_type("B", 1));

	auto it = myMap.find("A");



	B myB(1, 2);
	cout << "B.one: " << myB.one << endl;
	cout << "B.two: " << myB.two << endl;
	cout << "type:" << typeid(myB.two).name() << endl;

	cout << "size_t(false)=" << size_t(false) << " size_t(true)=" << size_t(true) << endl;

	cout << "Try D" << endl;
	D d;
	d.sz();

	cout << "Try D2" << endl;
	D2 c;
	c.sz();


    return 0;
}

