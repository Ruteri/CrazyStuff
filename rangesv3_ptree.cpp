#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <utility>
#include <vector>
#include <range/v3/all.hpp>

namespace pt = boost::property_tree;

template <typename KeyType, typename DataType = std::string>
struct basic_ptreev3 {
	using key_type = KeyType;
	using path_type = KeyType;
	using data_type = DataType;
	using self_type = basic_ptreev3<KeyType>;

	std::vector<std::pair<KeyType, self_type>> m_children {};
	using const_iterator = typename std::vector<std::pair<KeyType, self_type>>::const_iterator;
	// const_iterator begin_it;
	// const_iterator end_it;

	const_iterator begin() const { return m_children.begin(); }
	const_iterator end() const { return m_children.end(); }

	data_type m_data;
	
	basic_ptreev3() = default;
	basic_ptreev3(const data_type& data): m_data(data) {}

	/* serialization (access) */
	template <typename Str>
	Str get_value() const { return m_data; }

	bool empty() const { return m_children.size() == 0; }

	size_t count(const key_type& key) const {
		size_t count;
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
};

using ptreev3 = basic_ptreev3<std::string>;


#include <iostream>


int main() {

	std::vector<std::pair<std::string, std::string>> v{{"a","A"}, {"b","B"}};
	ptreev3 pt2;
	ptreev3 pt3;
	pt3.put_value("bar");
	pt2.put_child("foo", pt3);
	pt2.put("foo2", "baz");
	std::ostringstream buf2; 
	pt::write_json (buf2, pt2, false);
	std::cout << buf2.str();

	return 0;
}
