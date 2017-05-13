#include <cassert>
#include <functional>
#include <algorithm>
#include <iostream>

struct Base {
};

struct A: public Base {
	A(int a): name("struct A"), a(a) {}

	std::string name;
	int a;

	void print() const { std::cout << this << name << a << std::endl; }
};

struct B: public Base {
	B(double b): name("struct B"), b(b) {}

	std::string name;
	double b;

	void print() const { std::cout << this << name << b << std::endl; }
};

template <template <typename T,typename> class Container = std::vector, typename T = void, typename... Ts>
struct CompositeVector {
	CompositeVector(Container<T, std::allocator<T>>& t, Container<Ts, std::allocator<Ts>>&... ts): container(t), composite(ts...) {}

	Container<T, std::allocator<T>> container;
	CompositeVector<Container, Ts...> composite;

	void visit(const std::function<void(const T&)>& f, const std::function<void(const Ts&)>... fs) const {
		std::for_each(container.cbegin(), container.cend(), f);
		composite.visit(fs...);
	}
};

template <template <typename,typename> class Container>
struct CompositeVector<Container, void> {
	void visit() const {}
};

#include <vector>
#include <list>
#include <deque>

template <typename T, typename A = std::allocator<T>>
using container = std::list<T, A>;

int main() {

	container<A> vecA;
	//vecA.reserve(20);

	container<B> vecB;
	//vecB.reserve(20);

	std::vector<A*> vecA2;
	std::vector<B*> vecB2;

	for (int i = 0; i < 20; ++i) {
		for (int j = 32001; j > 0; --j) vecA2.push_back(new A(j));
		vecA.push_back(*vecA2[32001*i + i]);
	}

	for (int i = 0; i < 20; ++i) {
		for (int j = 32001; j > 0; --j) vecB2.push_back(new B(0.33*j));
		vecB.push_back(*vecB2[32001*i + i]);
	}

	const auto vec = CompositeVector<container, A, B>(vecA, vecB);

	const auto al = [](const A& a)->void{ a.print(); };
	const auto bl = [](const B& b)->void{ b.print(); };

	for (auto i = 50000; i > 0; --i)
		vec.visit(al, bl);

	return 0;
}
