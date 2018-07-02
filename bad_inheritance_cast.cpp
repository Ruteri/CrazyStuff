#include <cassert>

class Interface {
public:
	virtual int check(int a) { return 1; }
	virtual int check(double b) { return 2; }
};

class Derived: public Interface {
public:
	// override only one of the virtual "check" functions and this will blow up
	int check(double a) override { return 3; }
};

class UDerived: public Interface {
public:
	using Interface::check; // No way to specify whick version of check we want to pull
	int check(double a) override { return 4; }
};

int main() {
	Derived d;

	assert(d.check(1.0) == 3);
	assert(d.check(1) == 3); // OOPS

	UDerived ud;

	assert(ud.check(1.0) == 4);
	assert(ud.check(1) == 1);

	return 0;
}
