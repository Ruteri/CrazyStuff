#include <cassert>
#include <functional>
#include <algorithm>
#include <memory>

template <typename... Ts>
struct Visitor {
	Visitor(std::function<void(const Ts&)>... args): ts { std::move(args)... } {}

	template <typename T>
	void visit(const T& t) {
		return std::get<std::function<void(const T&)>>(ts)(t);
	}

private:
	std::tuple<std::function<void(const Ts&)>...> ts;

};

using BaseVisitor = Visitor<struct A, struct B>;

struct Base {
	virtual void accept(BaseVisitor& v) = 0;
};

struct A: public Base {
	void accept(BaseVisitor& v) override { return v.visit(*this); }
};

struct B: public Base {
	void accept(BaseVisitor& v) override { return v.visit(*this); }
};


int main() {
	std::shared_ptr<Base> base_ptr = std::make_shared<A>();

	std::string call;
	auto v = BaseVisitor([&call](const A&){ call = "A"; }, [&call](const B&){ call = "B"; });
	base_ptr->accept(v);

	assert( call == "A" );

	return 0;
}
