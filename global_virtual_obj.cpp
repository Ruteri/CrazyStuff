#include <iostream>
#include <cassert>

using namespace std;

struct A {
	static A* obj;
	static A* getObj() {
		return A::obj;
	}

	virtual int vcheck() { return 1; }
};

struct B: A {
	int vcheck() override { return 2; }
};

struct C: B {
	int vcheck() override { return 3; }
	int rcheck() { return 3; }
};


A* A::obj = nullptr;


int main() {
	/* init global AND hold alias pointer to use non-virtual functions of C */
	/* this is not really good, just a proof-of-concept for legacy code */
	C* c = new C();
	A::obj = c;

	assert(A::getObj()->vcheck() == 3);
	assert(B::getObj()->vcheck() == 3);
	assert(C::getObj()->vcheck() == 3);
	assert(c->rcheck() == 3);

	return 0;
}
