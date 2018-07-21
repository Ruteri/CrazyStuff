#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/json_parser/detail/write.hpp>

#include <utility>
#include <vector>
#include <range/v3/all.hpp>

namespace pt = boost::property_tree;

template <typename KeyType, typename DataType = std::string>
struct basic_ptree_copybased {
	using key_type = KeyType;
	using path_type = KeyType;
	using data_type = DataType;
	using self_type = basic_ptree_copybased<KeyType>;

	std::vector<std::pair<KeyType, self_type>> m_children {};
	using const_iterator = typename std::vector<std::pair<KeyType, self_type>>::const_iterator;
	// const_iterator begin_it;
	// const_iterator end_it;

	const_iterator begin() const { return m_children.begin(); }
	const_iterator end() const { return m_children.end(); }

	data_type m_data;
	
	basic_ptree_copybased() = default;
	basic_ptree_copybased(const data_type& data): m_data(data) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return m_data; }

	bool empty() const { return m_children.size() == 0; }

	size_t count(const key_type& key) const {
		size_t count = 0;
		for (const auto& c: m_children) {
			if (c.first == key) count++;
		}
		return count;
	}

	size_t size() const { return m_children.size(); }

	/* parsing (modifiers) */
	void put_value(const DataType &value) {
		m_data = value;
	}

	self_type &put_child(const path_type &path, const self_type &value) {
		m_children.push_back({path, value});
		return m_children.back().second;
	}

	self_type &put(const path_type &path, const DataType &value) {
		self_type new_node(value);
		return put_child(path, new_node);
	}

	void clear() {
		m_data = data_type();
		m_children.clear();
	}
};

using ptree_copybased = basic_ptree_copybased<std::string>;

#include <deque>

template <typename KeyType, typename DataType = std::string>
struct basic_ptree_iterator_based { // TODO: make container type a template
	using key_type = KeyType;
	using path_type = KeyType;
	using data_type = DataType;
	using self_type = basic_ptree_iterator_based<KeyType>;

	using container_type = typename std::deque<std::pair<KeyType, self_type>>;
	using const_iterator = typename container_type::const_iterator;

	// static (or better static inline constexpr from c++17)
	container_type empty_container;
	const_iterator begin_it = empty_container.begin();
	const_iterator end_it = empty_container.end();

	const_iterator begin() const { return begin_it; }
	const_iterator end() const { return end_it; }

	data_type m_data;
	
	basic_ptree_iterator_based() = default;
	basic_ptree_iterator_based(const data_type& data): m_data(data) {}
	basic_ptree_iterator_based(const_iterator _b, const_iterator _e, const data_type& data = data_type()): begin_it(_b), end_it(_e), m_data(data) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return m_data; }

	bool empty() const { return size() == 0; }

	size_t count(const key_type& key) const {
		size_t count = 0;
		for (auto it = begin_it; it != end_it; ++it) {
			if (it->first == key) count++;
		}
		return count;
	}

	size_t size() const { return std::distance(begin_it, end_it); }

	/* modifiers */
	void put_value(const DataType &value) {
		m_data = value;
	}

	void put_children(const const_iterator _b, const const_iterator _e) {
		begin_it = _b;
		end_it = _e;
	}

	void clear() {
		m_data = data_type();
		begin_it = empty_container.begin();
		end_it = empty_container.end();
	}
};

using ptree_iterator_based = basic_ptree_iterator_based<std::string>;


#include <iostream>

struct Alarm {
	using key_type = std::string;
	size_t id;
	size_t raise_time;
	std::string msg;
};

struct AlarmSource {
	std::string name;
	std::deque<std::pair<std::string, Alarm>> alarms;
};

struct te_variant_ptree_holder {
	using key_type = std::string;
    virtual void write_json_helper(std::basic_ostream<key_type::value_type> &stream, int indent, bool pretty) const = 0;

    virtual bool verify_json(int) const = 0;
};

template <typename T>
struct variant_ptree_holder: public te_variant_ptree_holder {
	using te_variant_ptree_holder::key_type;
    void write_json_helper(std::basic_ostream<key_type::value_type> &stream, int indent, bool pretty) const override {
		boost::property_tree::json_parser::write_json_helper(stream, obj, indent, pretty);
	}

    bool verify_json(int indent) const override {
		return boost::property_tree::json_parser::verify_json(obj, indent);
	}

	T& obj;

	/* template <typename... Args> // perfect forwarding maybe?
	variant_ptree_holder(Args... args): obj(T(args...)) {} // use by-value (T obj) */

	variant_ptree_holder(T& _obj): obj(_obj) {}

};

// adapter class
struct basic_alarm_ptree { // TODO: make container type a template
	using key_type = std::string;
	using data_type = Alarm;

	using container_type = typename std::deque<std::pair<key_type, Alarm>>;
	using const_iterator = typename container_type::const_iterator;

	boost::optional<std::reference_wrapper<AlarmSource>> alarm_source;
	container_type empty_container;

	const_iterator begin() const {
		if (alarm_source)
			return (*alarm_source).get().alarms.begin();
		return  empty_container.begin();
	}

	const_iterator end() const {
		if (alarm_source)
			return (*alarm_source).get().alarms.end();
		return  empty_container.end();
	}
	
	basic_alarm_ptree() = default;
	basic_alarm_ptree(AlarmSource& as): alarm_source(std::ref(as)) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return ""; }

	bool empty() const { return size() == 0; }

	size_t count(const key_type& key) const {
		return key.empty()? size() : 0;
	}

	size_t size() const { return alarm_source? (*alarm_source).get().alarms.size() : 0; }
};

#include <functional>

// ignore this, workaround for key_type in boost's function specialization
namespace std {

template <>
struct _Reference_wrapper_base<te_variant_ptree_holder>
    : _Reference_wrapper_base_impl<
      __has_argument_type<te_variant_ptree_holder>::value,
      __has_first_argument_type<te_variant_ptree_holder>::value
      && __has_second_argument_type<te_variant_ptree_holder>::value,
      te_variant_ptree_holder>
{
	using key_type = std::string;
};

} // ns std

struct basic_ptree_holder { // TODO: make container type a template
	using key_type = std::string;
	using path_type = std::string;
	using data_type = std::string;

	using held_type = std::reference_wrapper<te_variant_ptree_holder>;
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
		stream << Str(pt.msg);

		if (pretty) stream << Ch('\n');

		if (pretty) stream << Str(4 * indent, Ch(' '));
		stream << Ch('}');
    }

    template <>
    bool verify_json(const Alarm &, int) { return true; }

    template <>
    void write_json_helper(std::basic_ostream<std::string::value_type> &stream, 
                           const std::reference_wrapper<te_variant_ptree_holder> &pt,
                           int indent, bool pretty)
    {
		pt.get().write_json_helper(stream, indent, pretty);
	}

    template <>
    bool verify_json(const std::reference_wrapper<te_variant_ptree_holder> &pt, int indent) {
		return pt.get().verify_json(indent);
	}


}}} // ns boost::property_tree::json_parser


int main() {

	{ /*
		ptree_copybased pt2;
		ptree_copybased pt3;
		pt3.put_value("bar");
		pt2.put_child("foo", pt3);
		auto& pt4 = pt2.put("foo2", "baz");
		pt4.clear();
		pt4.put("foo3", "baz");
		std::ostringstream buf; 
		pt::write_json (buf, pt2, true);
		std::cout << buf.str();
	*/ }


	{ /*
		ptree_iterator_based pt;
		std::deque<std::pair<std::string, ptree_iterator_based>> v1{{"foo", ptree_iterator_based("bar")}};
		std::deque<std::pair<std::string, ptree_iterator_based>> v2{{"foo", ptree_iterator_based("baz")}};
		v1.push_back({"foo2", ptree_iterator_based(v2.begin(), v2.end())});
		pt.put_children(v1.begin(), v1.end());
		std::ostringstream buf; 
		pt::write_json (buf, pt, true);
		std::cout << buf.str();
	*/ }


	{
		basic_ptree_holder holder;
		AlarmSource as1;
		as1.alarms.emplace_back("a", Alarm{1, 100, "alarm1"});
		as1.alarms.emplace_back("b", Alarm{2, 113, "alarm2"});

		AlarmSource as2;
		as2.alarms.emplace_back("c", Alarm{1, 103, "alarm1"});
		as2.alarms.emplace_back("d", Alarm{2, 121, "alarm2"});

		AlarmSource as3;

		basic_alarm_ptree ap1(as1);
		variant_ptree_holder<basic_alarm_ptree> vp1(ap1);
		holder.put_child("alarm source 1", vp1);

		basic_alarm_ptree ap2(as2);
		variant_ptree_holder<basic_alarm_ptree> vp2(ap2);
		holder.put_child("alarm source 1", vp2);

		basic_alarm_ptree ap3(as3);
		variant_ptree_holder<basic_alarm_ptree> vp3(ap3);
		holder.put_child("alarm source 1", vp3);

		pt::ptree pt4;
		variant_ptree_holder<pt::ptree> vp4(pt4);
		holder.put_child("some other json stuff", vp4);
		pt4.put_value("some other value");

		pt::write_json(std::cout, holder, true);
	}
	
	{

	}

	return 0;
}
