#pragma once

#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <stdexcept>

#include "string_processing.h"
#include "document.h"
#include "read_input_functions.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;

enum class DocumentStatus {
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED,
};

class SearchServer {
public:
	explicit SearchServer(const std::string& stop_words);

	template <typename StopWords>
	explicit SearchServer(const StopWords& stop_words)
		: stop_words_(GetValidWordsSet(stop_words))
	{}

	void AddDocument(int document_id, const std::string& document,
		DocumentStatus status, const std::vector<int>& ratings);

	template <typename Predicate>
	std::vector<Document> FindTopDocuments(const std::string& raw_query,
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

	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus doc_status) const;

	std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

	size_t GetDocumentCount() const;

	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
		int document_id) const;

	int GetDocumentId(int index) const;

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};
	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};
	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};
	const std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, DocumentData> documents_;
	std::vector<int> document_ids_;

	template <typename Words>
	static std::set<std::string> GetValidWordsSet(const Words& words) {
		std::set<std::string> result;
		for (const std::string& word : words) {
			if (!IsValidWord(word)) {
				throw std::invalid_argument("Invalid word: " + word);
			}
			result.insert(word);
		}
		return result;
	}

	static bool IsValidWord(const std::string& word);

	static int ComputeAverageRating(const std::vector<int>& ratings);

	bool IsStopWord(const std::string& word) const;

	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

	QueryWord ParseQueryWord(std::string text) const;

	Query ParseQuery(const std::string& text) const;

	double ComputeWordInverseDocumentFreq(const std::string& word) const;

	template <typename Predicate>
	std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const {
		std::map<int, double> document_to_relevance;
		for (const std::string& word : query.plus_words) {
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

		for (const std::string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}

		std::vector<Document> matched_documents;
		for (const auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back(
				{ document_id, relevance, documents_.at(document_id).rating });
		}
		return matched_documents;
	}
};