#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();
	return result;
}

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;
	for (const char c : text) {
		if (c == ' ') {
			words.push_back(word);
			word = "";
		}
		else {
			word += c;
		}
	}
	words.push_back(word);

	return words;
}

struct Document {
	int id;
	double relevance;
	int rating;
};

enum class DocumentStatus {
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED,
};

class SearchServer {
public:
	void SetStopWords(const string& text) {
		for (const string& word : SplitIntoWords(text)) {
			stop_words_.insert(word);
		}
	}

	void AddDocument(int document_id, const string& document,
		DocumentStatus status, const vector<int>& ratings) {
		const vector<string> words = SplitIntoWordsNoStop(document);
		const double inv_word_count = 1.0 / words.size();
		for (const string& word : words) {
			word_to_document_freqs_[word][document_id] += inv_word_count;
		}
		documents_.emplace(document_id,
			DocumentData{ ComputeAverageRating(ratings), status });
	}

	template <typename Predicate>
	vector<Document> FindTopDocuments(const string& raw_query,
		Predicate predicate) const {
		const Query query = ParseQuery(raw_query);
		auto matched_documents = FindAllDocuments(query, predicate);

		sort(matched_documents.begin(), matched_documents.end(),
			[](const Document& lhs, const Document& rhs) {
				if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
					return lhs.rating > rhs.rating;
				}
				else {
					return lhs.relevance > rhs.relevance;
				}
			});
		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
		return matched_documents;
	}

	vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus doc_status) const {
		return FindTopDocuments(raw_query, [doc_status](int document_id, DocumentStatus status, int rating) {
			return status == doc_status;
			});
	}

	vector<Document> FindTopDocuments(const string& raw_query) const {
		return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}

	int GetDocumentCount() const { return documents_.size(); }

	tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
		int document_id) const {
		const Query query = ParseQuery(raw_query);
		vector<string> matched_words;
		for (const string& word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			if (word_to_document_freqs_.at(word).count(document_id)) {
				matched_words.push_back(word);
			}
		}
		for (const string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			if (word_to_document_freqs_.at(word).count(document_id)) {
				matched_words.clear();
				break;
			}
		}
		return { matched_words, documents_.at(document_id).status };
	}

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};

	set<string> stop_words_;
	map<string, map<int, double>> word_to_document_freqs_;
	map<int, DocumentData> documents_;

	bool IsStopWord(const string& word) const {
		return stop_words_.count(word) > 0;
	}

	vector<string> SplitIntoWordsNoStop(const string& text) const {
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			if (!IsStopWord(word)) {
				words.push_back(word);
			}
		}
		return words;
	}

	static int ComputeAverageRating(const vector<int>& ratings) {
		int rating_sum = 0;
		for (const int rating : ratings) {
			rating_sum += rating;
		}
		return rating_sum / static_cast<int>(ratings.size());
	}

	struct QueryWord {
		string data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(string text) const {
		bool is_minus = false;
		// Word shouldn't be empty
		if (text[0] == '-') {
			is_minus = true;
			text = text.substr(1);
		}
		return { text, is_minus, IsStopWord(text) };
	}

	struct Query {
		set<string> plus_words;
		set<string> minus_words;
	};

	Query ParseQuery(const string& text) const {
		Query query;
		for (const string& word : SplitIntoWords(text)) {
			const QueryWord query_word = ParseQueryWord(word);
			if (!query_word.is_stop) {
				if (query_word.is_minus) {
					query.minus_words.insert(query_word.data);
				}
				else {
					query.plus_words.insert(query_word.data);
				}
			}
		}
		return query;
	}

	// Existence required
	double ComputeWordInverseDocumentFreq(const string& word) const {
		return log(GetDocumentCount() * 1.0 /
			word_to_document_freqs_.at(word).size());
	}

	template <typename Predicate>
	vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const {
		map<int, double> document_to_relevance;
		for (const string& word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
			for (const auto [document_id, term_freq] :
				word_to_document_freqs_.at(word)) {
				const DocumentData& doc = documents_.at(document_id);
				if (predicate(document_id, doc.status, doc.rating)) {
					document_to_relevance[document_id] +=
						term_freq * inverse_document_freq;
				}
			}
		}

		for (const string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}

		vector<Document> matched_documents;
		for (const auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back(
				{ document_id, relevance, documents_.at(document_id).rating });
		}
		return matched_documents;
	}
};

void PrintDocument(const Document& document) {
	cout << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s << endl;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
	const string& func, unsigned line, const string& hint) {
	if (t != u) {
		cout << boolalpha;
		cout << file << "("s << line << "): "s << func << ": "s;
		cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
		cout << t << " != "s << u << "."s;
		if (!hint.empty()) {
			cout << " Hint: "s << hint;
		}
		cout << endl;
		abort();
	}
}

template <typename T>
void AssertImpl(const T& t, const string& t_str, const string& file, const string& func, unsigned line, const string& hint) {
	if (t != true) {
		cout << file << "("s << line << "): "s << func << ": "s;
		cout << "ASSERT("s << t_str << ") failed.";
		if (!hint.empty()) {
			cout << " Hint: " << hint;
		}
		cout << endl;
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

void TestExcludeStopWordsFromAddedDocumentContent() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };

	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc_id);
	}

	{
		SearchServer server;
		server.SetStopWords("in the"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		ASSERT(server.FindTopDocuments("in"s).empty());
	}
}

void TestAddDocuments() {
	const int doc0_id = 1;
	const string content0 = "black white red yellow"s;
	const vector<int> ratings0 = { 1, 2, 3 };
	const int doc1_id = 2;
	const string content1 = "black blue green purple"s;
	const vector<int> ratings1 = { 1, 2, 3 };

	SearchServer server;
	ASSERT_EQUAL(server.GetDocumentCount(), 0);
	server.AddDocument(doc0_id, content0, DocumentStatus::ACTUAL, ratings0);
	ASSERT_EQUAL(server.GetDocumentCount(), 1);
	server.AddDocument(doc1_id, content1, DocumentStatus::ACTUAL, ratings1);
	ASSERT_EQUAL(server.GetDocumentCount(), 2);
}

void TestMinusWords() {
	const int doc0_id = 1;
	const string content0 = "black white red yellow"s;
	const vector<int> ratings0 = { 1, 2, 3 };
	const int doc1_id = 2;
	const string content1 = "black blue green purple"s;
	const vector<int> ratings1 = { 1, 2, 3 };

	{
		SearchServer server;
		server.AddDocument(doc0_id, content0, DocumentStatus::ACTUAL, ratings0);
		server.AddDocument(doc1_id, content1, DocumentStatus::ACTUAL, ratings1);
		const auto found_docs = server.FindTopDocuments("black -blue"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc0_id);
	}
}

void TestMatchDocuments() {
	const int doc_id = 1;
	const string content = "black white red green blue"s;
	const vector<int> ratings = { 1, 2, 3 };

	SearchServer server;
	server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

	{
		const auto [words, status] = server.MatchDocument("grass is green"s, doc_id);
		ASSERT_EQUAL(words.size(), 1);
		ASSERT_EQUAL(words[0], "green"s);
	}

	{
		const auto [words, status] = server.MatchDocument("black -white"s, doc_id);
		ASSERT_EQUAL(words.size(), 0);
	}
}

void TestRelevanceCalculationAndSorting() {
	const double epsilon = 0.01;
	const double expected_relevance1 = 0.08;
	const double expected_relevance2 = 0.65;
	const double expected_relevance3 = 0.27;
	const vector<int> ratings = { 1, 2, 3 };
	const int doc0_id = 1;
	const string content0 = "white cat and fashion collar"s;
	const int doc1_id = 2;
	const string content1 = "fluffy cat fluffy tail"s;
	const int doc2_id = 3;
	const string content2 = "well-groomed dog expressive eyes"s;

	{
		SearchServer server;
		server.AddDocument(doc0_id, content0, DocumentStatus::ACTUAL, ratings);
		server.AddDocument(doc1_id, content1, DocumentStatus::ACTUAL, ratings);
		server.AddDocument(doc2_id, content2, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("fluffy well-groomed cat"s);
		ASSERT_EQUAL(found_docs.size(), 3);
		ASSERT_EQUAL(found_docs[0].id, 2);
		ASSERT_EQUAL(found_docs[1].id, 3);
		ASSERT_EQUAL(found_docs[2].id, 1);

		ASSERT(abs(found_docs[0].relevance - expected_relevance2) < epsilon);
		ASSERT(abs(found_docs[1].relevance - expected_relevance3) < epsilon);
		ASSERT(abs(found_docs[2].relevance - expected_relevance1) < epsilon);
	}
}

void TestRatingCalculation() {
	const int doc_id = 1;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };

	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_EQUAL(found_docs[0].rating, 2);
	}
}

void TestDocumentFiltering() {
	const int doc0_id = 1;
	const string content0 = "white cat and fashion collar"s;
	const vector<int> ratings0 = { 1, 1, 1 };
	const int doc1_id = 2;
	const string content1 = "fluffy cat fluffy tail"s;
	const vector<int> ratings1 = { 2, 2, 2 };
	const int doc2_id = 3;
	const string content2 = "well-groomed dog expressive eyes"s;
	const vector<int> ratings2 = { 3, 3, 3 };

	SearchServer server;
	server.AddDocument(doc0_id, content0, DocumentStatus::ACTUAL, ratings0);
	server.AddDocument(doc1_id, content1, DocumentStatus::BANNED, ratings1);
	server.AddDocument(doc2_id, content2, DocumentStatus::IRRELEVANT, ratings2);

	{
		const auto found_docs = server.FindTopDocuments("well-groomed cat"s, [](int doc_id, DocumentStatus status, int rating) {
			return doc_id > 0 && rating > 1 && status != DocumentStatus::REMOVED;
			});
		ASSERT_EQUAL(found_docs.size(), 2);
		ASSERT_EQUAL(found_docs[0].id, 3);
		ASSERT_EQUAL(found_docs[1].id, 2);
	}

	{
		const auto found_docs = server.FindTopDocuments("well-groomed cat"s, [](int doc_id, DocumentStatus status, int rating) {
			return doc_id != 2 && rating > 0 && status != DocumentStatus::BANNED;
			});
		ASSERT_EQUAL(found_docs.size(), 2);
		ASSERT_EQUAL(found_docs[0].id, 3);
		ASSERT_EQUAL(found_docs[1].id, 1);
	}

	{
		const auto found_docs = server.FindTopDocuments("well-groomed cat"s, [](int doc_id, DocumentStatus status, int rating) {
			return doc_id == 1 && rating == 1 && status == DocumentStatus::ACTUAL;
			});
		ASSERT_EQUAL(found_docs.size(), 1);
		ASSERT_EQUAL(found_docs[0].id, 1);
	}
}

void TestSearchDocumentsWithStatus() {
	const int doc0_id = 1;
	const string content0 = "white cat and fashion collar"s;
	const vector<int> ratings0 = { 1, 1, 1 };
	const int doc1_id = 2;
	const string content1 = "fluffy cat fluffy tail"s;
	const vector<int> ratings1 = { 2, 2, 2 };
	const int doc2_id = 3;
	const string content2 = "well-groomed dog expressive eyes"s;
	const vector<int> ratings2 = { 3, 3, 3 };

	SearchServer server;
	server.AddDocument(doc0_id, content0, DocumentStatus::ACTUAL, ratings0);
	server.AddDocument(doc1_id, content1, DocumentStatus::BANNED, ratings1);
	server.AddDocument(doc2_id, content2, DocumentStatus::IRRELEVANT, ratings2);

	const auto found_docs = server.FindTopDocuments("well-groomed cat"s, DocumentStatus::ACTUAL);
	ASSERT_EQUAL(found_docs.size(), 1);
	ASSERT_EQUAL(found_docs[0].id, 1);
}

void TestSearchServer() {
	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
	RUN_TEST(TestAddDocuments);
	RUN_TEST(TestMinusWords);
	RUN_TEST(TestMatchDocuments);
	RUN_TEST(TestRelevanceCalculationAndSorting);
	RUN_TEST(TestRatingCalculation);
	RUN_TEST(TestDocumentFiltering);
	RUN_TEST(TestSearchDocumentsWithStatus);
}

int main() {
	TestSearchServer();
	cout << "Search server testing finished"s << endl;

	return 0;
}