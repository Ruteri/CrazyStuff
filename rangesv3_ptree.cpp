#include <sstream>

// boost json writer patched for ranges v3 library operator-> not working
#include "boost_patches/write.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
// #include <boost/property_tree/json_parser/detail/write.hpp>

#include <utility>
#include <vector>
#include <range/v3/all.hpp>

#include <deque>
#include <iostream>

namespace pt = boost::property_tree;

struct Alarm {
	using key_type = std::string;
	size_t id;
	size_t raise_time;
	std::string msg;

	// non copyable
	Alarm(size_t _id, size_t _raise_time, std::string _msg): id(_id), raise_time(_raise_time), msg(_msg) {}
	Alarm(const Alarm&) = delete;
	Alarm(Alarm&&) = default;
	Alarm& operator=(const Alarm&) = delete;
	Alarm& operator=(Alarm&&) = default;
};

struct AlarmSource {
	std::string name;
	std::deque<Alarm> alarms;
};

struct te_multitype_ptree_holder {
	using key_type = std::string;
    virtual void write_json_helper(std::basic_ostream<key_type::value_type> &stream, int indent, bool pretty) const = 0;

    virtual bool verify_json(int) const = 0;
};

template <typename T>
struct multitype_ptree_holder: public te_multitype_ptree_holder {
	using te_multitype_ptree_holder::key_type;
    void write_json_helper(std::basic_ostream<key_type::value_type> &stream, int indent, bool pretty) const override {
		boost::property_tree::json_parser::write_json_helper(stream, obj, indent, pretty);
	}

    bool verify_json(int indent) const override {
		return boost::property_tree::json_parser::verify_json(obj, indent);
	}

	T& obj;

	/* template <typename... Args> // perfect forwarding maybe?
	// this should be enable_if'd for types you want to copy / construct inplace
	multitype_ptree_holder(Args... args): obj(T(args...)) {} // use by-value (T obj) */

	multitype_ptree_holder(T& _obj): obj(_obj) {}

};

template <class Range>
struct basic_range_ptree { 
	using key_type = std::string;
	using data_type = std::string;

	Range &range;
	using const_iterator = decltype(range.begin());

	const_iterator begin() const {
		return range.begin();
	}

	const_iterator end() const {
		return range.end();
	}
	
	data_type m_data;

	basic_range_ptree(Range &_range, const data_type& data = data_type()): range(_range), m_data(data) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return Str(m_data); }

	bool empty() const { return size() == 0; }

	size_t count(const key_type& key) const {
		return key.empty()? size() : 0;
	}

	// see errors section
	size_t size() const { return std::distance(begin(), end()); }
};

#include <functional>

// ignore this, workaround for key_type in boost's function specialization
// best to write your own reference wrapper for this specific need
/*
template <typename T>
struct my_wrapper {
	reference_wrapper<T> ref;
	T& get() { return ref.get(); }
	using key_type = T::key_type;
};
*/
namespace std {

template <>
struct _Reference_wrapper_base<te_multitype_ptree_holder>
    : _Reference_wrapper_base_impl<
      __has_argument_type<te_multitype_ptree_holder>::value,
      __has_first_argument_type<te_multitype_ptree_holder>::value
      && __has_second_argument_type<te_multitype_ptree_holder>::value,
      te_multitype_ptree_holder>
{
	using key_type = std::string;
};

} // ns std

struct basic_ptree_holder { // TODO: make container type a template
	using key_type = std::string;
	using path_type = std::string;
	using data_type = std::string;

	using held_type = std::reference_wrapper<te_multitype_ptree_holder>;
	using container_type = typename std::deque<std::pair<key_type, held_type>>;
	using const_iterator = typename container_type::const_iterator;

	container_type m_children;

	const_iterator begin() const { return m_children.begin(); }
	const_iterator end() const { return m_children.end(); }
	
	data_type m_data;
	basic_ptree_holder(const data_type& data = data_type()): m_data(data) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return m_data; }

	bool empty() const { return m_children.empty(); }

	size_t count(const key_type& key) const {
		return key.empty()? size() : 0;
	}

	size_t size() const { return m_children.size(); }

	/* modifiers */
	void put_value(const data_type &value) {
		m_data = value;
	}

	template <typename T>
	void put_child(const path_type &path, T& value) {
		put_child(path, held_type { std::ref(value) });
	}

	void put_child(const path_type &path, const held_type &value) {
		m_children.push_back({path, value});
	}

	void put(const path_type &path, const held_type &value) {
		held_type new_node(value);
		put_child(path, new_node);
	}
};

namespace boost { namespace property_tree { namespace json_parser {
    template <>
    void write_json_helper(std::basic_ostream<std::string::value_type> &stream, 
                           const Alarm &pt,
                           int indent, bool pretty)
    {
		using Ch = char;
		using Str = std::string;

		stream << Ch('{');

		if (pretty) stream << Ch('\n') << Str(4 * (indent + 1), Ch(' '));
		stream << Ch('"') << Str("id") << Ch('"') << Ch(':');
		if (pretty) stream << Ch(' ');
		stream << pt.id << Ch(',');

		if (pretty) stream << Ch('\n') << Str(4 * (indent + 1), Ch(' '));
		stream << Ch('"') << Str("raise_time") << Ch('"') << Ch(':');
		if (pretty) stream << Ch(' ');
		stream << pt.raise_time << Ch(',');

		if (pretty) stream << Ch('\n') << Str(4 * (indent + 1), Ch(' '));
		stream << Ch('"') << Str("msg") << Ch('"') << Ch(':');
		if (pretty) stream << Ch(' ');
		stream << Ch('"') << Str(pt.msg) << Ch('"') ;

		if (pretty) stream << Ch('\n');

		if (pretty) stream << Str(4 * indent, Ch(' '));
		stream << Ch('}');
    }

    template <>
    bool verify_json(const Alarm &, int) { return true; }

    template <>
    void write_json_helper(std::basic_ostream<std::string::value_type> &stream, 
                           const std::reference_wrapper<te_multitype_ptree_holder> &pt,
                           int indent, bool pretty)
    {
		pt.get().write_json_helper(stream, indent, pretty);
	}

    template <>
    bool verify_json(const std::reference_wrapper<te_multitype_ptree_holder> &pt, int indent) {
		return pt.get().verify_json(indent);
	}


}}} // ns boost::property_tree::json_parser


int main() {

	{
		basic_ptree_holder holder;

		// adding good ol' ptree
		pt::ptree normal_ptree("regular ptree attr value");
		multitype_ptree_holder<decltype(normal_ptree)> vp(normal_ptree);
		holder.put_child("regular ptree attr", vp);

		// class Alarm is not copyable - this works on original references to alarms in AlarmSource / other containers which is actually, like, super useful
		// ranges from some holder via ranges (w/o copying the container or the elements)
		AlarmSource as1;
		as1.alarms.push_back(Alarm{1, 100, ""});
		as1.alarms.push_back(Alarm{2, 113, ""});

		AlarmSource as2;
		as2.alarms.emplace_back(Alarm{1, 103, ""});
		as2.alarms.emplace_back(Alarm{2, 121, ""});

		AlarmSource as3;

		auto alarm_to_json_pair = [](Alarm& a) -> std::pair<std::string, Alarm&> { return {"", a}; };
		auto alarm_older_than = [](int _min) { return [_min](Alarm& a){ return a.raise_time < _min; }; };

		size_t c_id = 1;
		auto fix_ids = [&c_id](std::pair<std::string, Alarm&> ap) { ap.second.id = c_id++; };

		// you can actually use everything from the ranges library to filter / modify objects
		auto && range = ranges::view::concat(as1.alarms, as2.alarms, as3.alarms)
		              | ranges::view::remove_if(alarm_older_than(110))
		              | ranges::view::transform(alarm_to_json_pair);

		ranges::for_each(range, fix_ids);


		basic_range_ptree<decltype(range)> rptree2(range);
		multitype_ptree_holder<decltype(rptree2)> vp2(rptree2);
		holder.put_child("alarms 1", vp2);

		// creating range from an array (or any other container)
		std::array<Alarm, 4> alarms_arr = {
			Alarm {1, 321, "msg 1"},
			Alarm {2, 363, "msg 2"},
			Alarm {3, 431, "msg 3"},
			Alarm {4, 1204, "msg 4"}
		};

		auto range2 = alarms_arr | ranges::view::transform(alarm_to_json_pair);
		basic_range_ptree<decltype(range2)> rptree(range2);
		multitype_ptree_holder<decltype(rptree)> vp6(rptree);
		holder.put_child("alarms 2", vp6);
		
		basic_ptree_holder holder2;
		holder2.put_value("You can even put holders inside other holders (like in normal ptree)");
		multitype_ptree_holder<basic_ptree_holder> vp3(holder2);
		holder.put_child("Other holder", vp3);

		pt::write_json(std::cout, holder, true);
	}

	{ // errors (in my opinion) I encountered in range-v3
		std::vector<int> v1 {0, 1, 2, 3};
		std::vector<int> v2 {0, 1, 2, 3};
		auto add_5 = [](int c) { return c + 5; };
		auto remove_if_less_than = [](int _min) { return [_min](int c) -> bool { return c < _min; }; };
		auto && range = ranges::view::concat(v1, v2) | ranges::view::remove_if(remove_if_less_than(2)) | ranges::view::transform(add_5);

		auto bit = ranges::begin(range);
		auto eit = ranges::begin(range);
		auto size = std::distance(bit, eit);
		// auto size = range.size() // Oops, doesn't compile (hence std::distance instead of .size() in range wrapper)

		auto int_to_pair = [](int c) -> std::pair<int, int> { return {c, c}; };
		auto range2 = v1 | ranges::view::transform(int_to_pair);
		auto bit2 = range2.begin();
		// auto si = bit2->second; // Oops, doesn't compile (see boost's write.hpp patch)
	}

	return 0;
}
