struct Visitable {
	virtual void accept(const struct AnyVisitor&) = 0;
};

#include <memory>
#include <stdexcept>
struct AnyVisitor {
	template <typename... T>
	AnyVisitor(std::function<void(T)>... fs): _visitor(new visitor_holder<T...>(fs...)) {}

private:
	struct base_holder {
		virtual ~base_holder() {} // we need one virtual function for dynamic_cast to work
	};

	template <typename T>
	struct base_type_holder: virtual base_holder {
		base_type_holder(std::function<void(T)> f): base_holder(), f(f) {}
		void visit(T&& t) const { f(std::forward<T>(t)); } // sadly cannot make this virtual
	private:
		std::function<void(T)> f;
	};

	template <typename... T> struct visitor_holder: base_type_holder<T>... {
		visitor_holder(std::function<void(T)>... fs): base_type_holder<T>(fs)... {}
	};

	std::shared_ptr<base_holder> _visitor;

public:
	// Function for accessing the templated constructor
	template <typename... T>
	static AnyVisitor createVisitor(std::function<void(T)>... fs) { return AnyVisitor(fs...); }

	// Function generated for each type that the visitor accesses
	template <class T>
	void visit(T&& t) const {
		auto inner_ptr = std::dynamic_pointer_cast<base_type_holder<T>>(_visitor);
		if (inner_ptr) inner_ptr->visit(std::forward<T>(t));
		else throw std::runtime_error("Visitor used with bad type");
	}
};


struct DerivedClass1: public Visitable
{
	void accept(const AnyVisitor& v) override { v.visit(*this); }
};

struct DerivedClass2: public Visitable
{
	void accept(const AnyVisitor& v) override { v.visit(*this); }
};


#include <vector>
#include <iostream>
int main() {

	std::function<void(DerivedClass1&)> fsd1 = [](DerivedClass1&){ std::cout << "c1" <<std::endl; };
	AnyVisitor v = AnyVisitor(
		fsd1,
		std::function<void(DerivedClass2&)>{[](DerivedClass2&){ std::cout << "c2" <<std::endl; }},
		std::function<void(DerivedClass2)>{[](DerivedClass2){ std::cout << "c2v" <<std::endl; }},
		std::function<void(int)>{[](int i) { std::cout << i << std::endl; }},
		std::function<void(std::string&)>{[](std::string& s) { std::cout << s << std::endl; }},
		std::function<void(std::string)>{[](std::string s) { std::cout << s << std::endl; }}
	);

	AnyVisitor v2 = AnyVisitor::createVisitor<DerivedClass1&, DerivedClass2&>(
		[](DerivedClass1&){ std::cout << "c12" <<std::endl; },
		[](DerivedClass2&){ std::cout << "c22" <<std::endl; }
	);

	v.visit(42);
	//int i = 2;
	//v.visit(i); // call with int& -> throws runtime_error
	std::string s("The answer to The Question is: ");
	v.visit(s); // by reference
	v.visit(std::string{"123"}); // by value
	std::vector<Visitable*> objs { new DerivedClass2(), new DerivedClass1(), new DerivedClass1() };
	for (auto o: objs) {
		o->accept(v); // always calls lambda with reference type ( visit(*this); )
		o->accept(v2); // always calls lambda with reference type ( visit(*this); )
	}
	v.visit(DerivedClass2{}); // by value

	
	fsd1 = std::function<void(DerivedClass1&)>{[](DerivedClass1&){ std::cout << "c12" <<std::endl; }};
	objs[2]->accept(v);

}
