#pragma once

#include <iostream>
#include <string>

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
	const std::string& func, unsigned line, const std::string& hint) {
	using namespace std::string_literals;
	if (t != u) {
		std::cerr << std::boolalpha;
		std::cerr << file << "("s << line << "): "s << func << ": "s;
		std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
		std::cerr << t << " != "s << u << "."s;
		if (!hint.empty()) {
			std::cerr << " Hint: "s << hint;
		}
		std::cerr << std::endl;
		abort();
	}
}

template <typename T>
void AssertImpl(const T& t, const std::string& t_str, const std::string& file, const std::string& func, unsigned line, const std::string& hint) {
	using namespace std::string_literals;
	if (t != true) {
		std::cerr << file << "("s << line << "): "s << func << ": "s;
		std::cerr << "ASSERT("s << t_str << ") failed.";
		if (!hint.empty()) {
			std::cout << " Hint: " << hint;
		}
		std::cerr << std::endl;
		abort();
	}
}

template <typename Func>
void RunTestImpl(Func f) {
	f();
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))
#define RUN_TEST(func)  { RunTestImpl(func); std::cerr << #func << " OK" << std::endl; }
