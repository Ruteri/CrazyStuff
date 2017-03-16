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

template <typename Base, typename T = void, typename... Ts>
struct CompositeVector {
	CompositeVector(std::vector<T> t, std::vector<Ts>... ts): vec(t), /*it(vec.end()),*/ composite(ts...) {}
	std::vector<T> vec;

	CompositeVector<Base, Ts...> composite;

	void visit(std::function<void(Base&)> f) {
		{
			for (auto it = vec.begin(); it != vec.end(); ++it) {
				f(*it);
			}
		}

		composite.visit(f);
	}
};

template <typename Base>
struct CompositeVector<Base, void> {
	void visit(std::function<void(Base&)>) {}
};

int main() {
	std::vector<A> vecA({A()});
	std::vector<B> vecB({B()});
	auto vec = CompositeVector<Base, A, B>(vecA, vecB);

	vec.visit([](Base& base){ base.print(); });

	return 0;
}
