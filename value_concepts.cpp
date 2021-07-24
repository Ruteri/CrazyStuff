#include <concepts>
#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>

/* Re-iteration on value polymorphism with C++20 */

/*
Concepts don't add too much to the type erasure, but the error messages are much better for types that fail to implement the draw function.
The code making use of DrawableEntry is also a bit more type-specific than what we'd usually get with type erasure (drawable_ptr is only enabled for types conforming to DrawableEntry)
*/


/* library part */

template <typename T>
void draw(const T &entry, std::ostream &out, int position);

template <typename T>
concept DrawableEntry = std::move_constructible<T> && std::copy_constructible<T> && requires(const T &t) {
	{ ::draw<T>(t, std::cout, 0) } -> std::same_as<void>;
};

template <typename T>
void draw(const T &entry, std::ostream &out, int position)
	requires requires (const T &t) { out << t; }
{
	out << std::string(position, ' ') << entry << '\n';
}

class drawable_ptr {
	struct drawable_ptr_api {
		virtual ~drawable_ptr_api() = default;
		virtual void draw(std::ostream &out, int position) const = 0;
		virtual std::unique_ptr<drawable_ptr_api> copy() const = 0;
	};

	template <DrawableEntry DE>
	struct drawable_ptr_te: drawable_ptr_api {
		DE _data;
		drawable_ptr_te(DE &&data): _data(std::move(data)) {}
		void draw(std::ostream &out, int position) const final { return ::draw(this->_data, out, position); }
		std::unique_ptr<drawable_ptr_api> copy() const final { return std::make_unique<drawable_ptr_te<DE>>(DE(this->_data)); }
	};

	std::unique_ptr<drawable_ptr_api> _ptr;

public:
	drawable_ptr(const drawable_ptr &other): _ptr(other._ptr->copy()) {}
	drawable_ptr(drawable_ptr &&other): _ptr(std::move(other)._ptr) {}

	template <DrawableEntry DE> /* pretty cool addition - helps with understanding the code, much better than duck-typed typename T, and also more consise and descriptive than enable_if */
	drawable_ptr(DE data): _ptr(std::make_unique<drawable_ptr_te<DE>>(std::move(data))) {}

	friend void draw<drawable_ptr>(const drawable_ptr &entry, std::ostream &out, int position);
};

template <>
void draw(const drawable_ptr &entry, std::ostream &out, int position) {
	entry._ptr->draw(out, position);
}

using document_t = std::vector<drawable_ptr>;

template <>
void draw(const document_t &document, std::ostream &out, int position) {
	out << std::string(position, ' ') << "<document>\n";
	for (const auto &entry: document) {
		draw(entry, out, position + 2);
	}
	out << std::string(position, ' ') << "</document>\n";
}


/* user code */

struct UserDefinedType {
	std::string value;
};

template <>
void draw(const UserDefinedType &v, std::ostream &out, int position) {
	out << std::string(position, ' ') << "</UserDefinedType value='" << v.value << std::string("'>\n");
}

int main() {
	document_t document;

	document.emplace_back(42);
	document.emplace_back(std::string("string entry"));
	document.emplace_back(UserDefinedType("user entry"));

	document_t document2 = document;
	document2.emplace_back(std::string("another string entry"));
	document.emplace_back(document2);

	document.emplace_back(std::string("yet another string entry"));

	std::stringstream ss;
	draw(document, ss, 0);

	assert(ss.str() == std::string(R"EOF(<document>
  42
  string entry
  </UserDefinedType value='user entry'>
  <document>
    42
    string entry
    </UserDefinedType value='user entry'>
    another string entry
  </document>
  yet another string entry
</document>
)EOF"));

	return 0;
}
