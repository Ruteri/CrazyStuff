#include <cassert>
#include <functional>
#include <algorithm>
#include <iostream>

struct Base {
	virtual void print() = 0;
};

struct A: public Base {
	void print() { std::cout << "struct A" << std::endl; }
};

struct B: public Base {
	void print() { std::cout << "struct B" << std::endl; }
};

template <typename T = void, typename... Ts>
struct CompositeVector {
	CompositeVector(std::vector<T> t, std::vector<Ts>... ts): vec(t), /*it(vec.end()),*/ composite(ts...) {}
	std::vector<T> vec;

	CompositeVector<Ts...> composite;

	template <typename CommonBase>
	void visit(std::function<void(CommonBase&)> f) {
		{
			for (auto it = vec.begin(); it != vec.end(); ++it) {
				f(*it);
			}
		}

		composite.visit(f);
	}
};

template <>
struct CompositeVector<void> {
	template <typename T>
	void visit(std::function<void(T&)>) {}
};

int main() {
	std::vector<A> vecA({A()});
	std::vector<B> vecB({B()});
	auto vec = CompositeVector<A, B>(vecA, vecB);

	vec.visit<Base>([](Base& base){ base.print(); });

	return 0;
}
