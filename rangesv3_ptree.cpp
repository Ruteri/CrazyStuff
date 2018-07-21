#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/json_parser/detail/write.hpp>

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

	return 0;
}
