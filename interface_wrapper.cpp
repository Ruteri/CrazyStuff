#include <iostream>

#include <type_traits>
namespace detail {
	template <typename... ts>
	struct any_true;

	template <typename t, typename... ts>
	struct any_true<t, ts...> {
		static const bool value = t::value || any_true<ts...>::value;
	};

	template <typename t>
	struct any_true<t> {
		static const bool value = t::value;
	};
} // detail


struct interface1 {
	virtual void act1() = 0;
	interface1* operator->() { return this; }
};

struct interface2 {
	virtual void act2() = 0;
};

struct interface3 {
	virtual void act3() { std::cout << "Act3!" << std::endl; }
};

struct handler1: interface1 {
	void act1() { std::cout << "Act1!" << std::endl; }
};

// Does not conform to interface
struct handler2 {
	void act2() { std::cout << "Act2!" << std::endl; }
};

struct handler2_wrapper: interface2 {
	handler2_wrapper(): base(new handler2()) {}

	void act2() { base->act2(); }	

private:
	handler2* base;
};

struct handler2_2: interface2 {
	void act2() { std::cout << "Act2_2!" << std::endl; }
};

struct handler3: interface3 {
	void act3() { std::cout << "Act3_2!" << std::endl; }
};


#include <functional>
template <typename interface>
struct _interface {
	using Args = interface*;

	_interface(interface* t): _impl(t) {}
	_interface(std::function<interface*(void)> f): _impl(f()) {}

	interface* operator->() { return _impl; }
	const interface* const operator->() const { return _impl; }

private:
	interface* _impl;
};

template <typename... ts>
struct interface_wrapper: virtual _interface<ts>... {
	interface_wrapper(typename _interface<ts>::Args... args): _interface<ts>(args)... {}

	template <typename it>
	typename std::enable_if<detail::any_true<std::is_same<ts, it>...>::value, _interface<it>&>::type
	get() {
		static_assert(detail::any_true<std::is_base_of<ts, it>...>::value, "");
		return *static_cast<_interface<it>*>(this);
	}
}; 


struct wrapper_user_passive {
	wrapper_user_passive(interface1* c1, interface3* c3): m_if(c1, c3) {}

	void call() { m_if.get<interface1>()->act1(); }

private:
	interface_wrapper<interface1, interface3> m_if;

};

#include <memory>
struct wrapper_user_active {
	wrapper_user_active(interface1* c1, bool m_c3): m_if(nullptr) {
		interface3* if3 = m_c3? new interface3() : new handler3();
		m_if.reset(new interface_wrapper<interface1, interface3>(c1, if3));
	}

	void call() { m_if->get<interface3>()->act3(); }

private:
	std::unique_ptr<interface_wrapper<interface1, interface3>> m_if;

};

struct handler_user {
	handler_user(interface1* c1): m_if1(*c1) {}
	handler_user(interface1& c1): m_if1(c1) {}

	void call() { m_if1->act1(); }

private:
	interface1& m_if1;

};


int main() {
	handler1 concrete1;
	handler2_wrapper concrete2;
	handler2_2 concrete2_2;

	interface_wrapper<interface1, interface2> m_if(&concrete1, &concrete2);

	m_if.get<interface1>()->act1();
	m_if.get<interface2>()->act2();

	auto m_if2 = interface_wrapper<interface2, interface3>(&concrete2_2, new interface3());

	m_if2.get<interface2>()->act2();
	m_if2.get<interface3>()->act3();

	interface3 concrete3;
	wrapper_user_passive w_user1(&concrete1, &concrete3);
	w_user1.call();

	wrapper_user_active w_user2(&concrete1, true);
	wrapper_user_active w_user3(&concrete1, false);

	w_user2.call();
	w_user3.call();

	handler_user h_user1(new handler1());
	h_user1.call();

	return 0;
}
