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

struct basic_alarm_ptree { // TODO: make container type a template
	using key_type = std::string;
	using data_type = Alarm;

	using container_type = typename std::deque<std::pair<key_type, Alarm>>;
	using const_iterator = typename container_type::const_iterator;

	boost::optional<AlarmSource&> alarm_source;
	container_type empty_container;

	const_iterator begin() const {
		if (alarm_source)
			return (*alarm_source).alarms.begin();
		return  empty_container.begin();
	}

	const_iterator end() const {
		if (alarm_source)
			return (*alarm_source).alarms.end();
		return  empty_container.end();
	}
	
	basic_alarm_ptree() = default;
	basic_alarm_ptree(AlarmSource& as): alarm_source(as) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return ""; }

	bool empty() const { return size() == 0; }

	size_t count(const key_type& key) const {
		return key.empty()? size() : 0;
	}

	size_t size() const { return alarm_source? (*alarm_source).alarms.size() : 0; }
};

struct basic_alarm_ptree_holder { // TODO: make container type a template
	using key_type = std::string;
	using data_type = basic_alarm_ptree;

	using container_type = typename std::deque<std::pair<key_type, data_type>>;
	using const_iterator = typename container_type::const_iterator;

	container_type alarm_sources;

	const_iterator begin() const { return alarm_sources.begin(); }
	const_iterator end() const { return alarm_sources.end(); }
	
	key_type m_data;
	basic_alarm_ptree_holder(const key_type& data = key_type()): m_data(data) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return m_data; }

	bool empty() const { return alarm_sources.empty(); }

	size_t count(const key_type& key) const {
		return key.empty()? size() : 0;
	}

	size_t size() const { return alarm_sources.size(); }

	// TODO: add modifiers
};

namespace boost { namespace property_tree { namespace json_parser {
    template <>
    void write_json_helper(std::basic_ostream<typename Alarm::key_type::value_type> &stream, 
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

}}} // ns boost::property_tree::json_parser



int main() {

	{
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
	}


	{
		ptree_iterator_based pt;
		std::deque<std::pair<std::string, ptree_iterator_based>> v1{{"foo", ptree_iterator_based("bar")}};
		std::deque<std::pair<std::string, ptree_iterator_based>> v2{{"foo", ptree_iterator_based("baz")}};
		v1.push_back({"foo2", ptree_iterator_based(v2.begin(), v2.end())});
		pt.put_children(v1.begin(), v1.end());
		std::ostringstream buf; 
		pt::write_json (buf, pt, true);
		std::cout << buf.str();
	}

	{
		AlarmSource as1;
		as1.alarms.emplace_back("", Alarm{1, 100, "alarm1"});
		as1.alarms.emplace_back("", Alarm{2, 113, "alarm2"});

		AlarmSource as2;
		as2.alarms.emplace_back("", Alarm{1, 103, "alarm1"});
		as2.alarms.emplace_back("", Alarm{2, 121, "alarm2"});

		basic_alarm_ptree_holder holder;
		holder.alarm_sources.push_back({"alarm source 1", basic_alarm_ptree(as1)});
		holder.alarm_sources.push_back({"alarm source 2", basic_alarm_ptree(as2)});
		pt::write_json(std::cout, holder, true);
	}
	

	return 0;
}
