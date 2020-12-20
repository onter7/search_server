#pragma once

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
	const string& func, unsigned line, const string& hint) {
	if (t != u) {
		cerr << boolalpha;
		cerr << file << "("s << line << "): "s << func << ": "s;
		cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
		cerr << t << " != "s << u << "."s;
		if (!hint.empty()) {
			cerr << " Hint: "s << hint;
		}
		cerr << endl;
		abort();
	}
}

template <typename T>
void AssertImpl(const T& t, const string& t_str, const string& file, const string& func, unsigned line, const string& hint) {
	if (t != true) {
		cerr << file << "("s << line << "): "s << func << ": "s;
		cerr << "ASSERT("s << t_str << ") failed.";
		if (!hint.empty()) {
			cout << " Hint: " << hint;
		}
		cerr << endl;
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
#define RUN_TEST(func)  { RunTestImpl(func); cerr << #func << " OK" << endl; }
