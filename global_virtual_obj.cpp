#include <iostream>
#include <cassert>

using namespace std;

struct A {
	virtual A* getPtr() {
		return A::obj;
	}

	static A* obj;
	static A* getObj() {
		return obj->getPtr();
	}

	virtual int vcheck() { return 1; }
};

struct B: A {
	B* getPtr() override {
		return (B*) A::obj;
	}

	int vcheck() override { return 2; }
};

struct C: B {
	C* getPtr() override {
		return (C*) A::obj;
	}

	int vcheck() override { return 3; }
	int rcheck() { return 3; }
};


A* A::obj = nullptr;


int main() {
	// init fn
	C* c = new C();
	A::obj = c;

	assert(A::getObj()->vcheck() == 3);
	assert(B::getObj()->vcheck() == 3);
	assert(C::getObj()->vcheck() == 3);
	assert(c->rcheck() == 3);

	return 0;
}
