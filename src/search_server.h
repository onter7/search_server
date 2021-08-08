#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <execution>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "concurrent_map.h"
#include "document.h"
#include "log_duration.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
	explicit SearchServer(std::string_view stop_words);

	explicit SearchServer(const std::string& stop_words);

	template <typename StopWords>
	explicit SearchServer(const StopWords& stop_words)
		: stop_words_(GetValidWordsSet(stop_words))
	{}

	void AddDocument(int document_id, std::string_view document,
		DocumentStatus status, const std::vector<int>& ratings);

	template <typename ExecutionPolicy, typename Predicate>
	std::vector<Document> FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query,
		Predicate predicate) const {
		const Query query = ParseQuery(raw_query);
		auto matched_documents = FindAllDocuments(policy, query, predicate);

		std::sort(policy, matched_documents.begin(), matched_documents.end(),
			[](const Document& lhs, const Document& rhs) {
				if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
					return lhs.rating > rhs.rating;
				}
				else {
					return lhs.relevance > rhs.relevance;
				}
			}
		);
		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
		return matched_documents;
	}

	template <typename Predicate>
	std::vector<Document> FindTopDocuments(std::string_view raw_query,
		Predicate predicate) const {
		return FindTopDocuments(std::execution::seq, raw_query, predicate);
	}

	std::vector<Document> FindTopDocuments(std::string_view raw_query,
		DocumentStatus doc_status) const;

	std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(ExecutionPolicy policy,
		std::string_view raw_query, DocumentStatus doc_status) const {
		return FindTopDocuments(policy, raw_query, [doc_status](int document_id, DocumentStatus status, int rating) {
			return status == doc_status;
		});
	}

	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query) const {
		return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
	}

	std::size_t GetDocumentCount() const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query,
		int document_id) const;

	template <typename ExecutionPolicy>
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy policy,
		std::string_view raw_query, int document_id) const {
		static_assert(std::is_same_v<ExecutionPolicy, std::execution::sequenced_policy> || std::is_same_v<ExecutionPolicy, std::execution::parallel_policy>);
		const Query query = ParseQuery(raw_query);

		const bool contains_minus_words = std::any_of(
			policy,
			query.minus_words.begin(),
			query.minus_words.end(),
			[this, document_id](const std::string_view word) {
				return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id);
			}
		);

		std::vector<std::string_view> matched_words;
		if (!contains_minus_words) {
			matched_words.resize(query.plus_words.size());			
			const auto it = std::copy_if(
				policy,
				query.plus_words.begin(),
				query.plus_words.end(),
				matched_words.begin(),
				[this, document_id](const std::string_view word) {
					return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id);
				}
			);
			matched_words.erase(it, matched_words.end());
		}

		return { matched_words, documents_.at(document_id).status };
	}

	std::set<int>::const_iterator begin() const;

	std::set<int>::const_iterator end() const;

	const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

	void RemoveDocument(int document_id);

	template <typename ExecutionPolicy>
	void RemoveDocument(ExecutionPolicy policy, int document_id) {
		static_assert(std::is_same_v<ExecutionPolicy, std::execution::sequenced_policy> || std::is_same_v<ExecutionPolicy, std::execution::parallel_policy>);
		const auto it = documents_.find(document_id);
		if (it == documents_.end()) return;
		std::for_each(
			policy,
			it->second.word_to_freq.begin(),
			it->second.word_to_freq.end(),
			[this, document_id](const std::pair<std::string_view, double>& pair) {
				word_to_document_freqs_.at(pair.first).erase(document_id);
			}
		);
		documents_.erase(document_id);
		document_ids_.erase(document_id);
	}

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
		std::set<std::string> words;
		std::map<std::string_view, double> word_to_freq;
	};
	struct QueryWord {
		std::string_view data;
		bool is_minus;
		bool is_stop;
	};
	struct Query {
		std::set<std::string_view> plus_words;
		std::set<std::string_view> minus_words;
	};
	const std::set<std::string, std::less<>> stop_words_;
	std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
	std::map<int, DocumentData> documents_;
	std::set<int> document_ids_;

	template <typename Words>
	static std::set<std::string, std::less<>> GetValidWordsSet(const Words& words) {
		std::set<std::string, std::less<>> result;
		for (std::string_view word : words) {
			if (!IsValidWord(word)) {
				throw std::invalid_argument("Invalid word: " + std::string(word));
			}
			result.insert(std::string(word));
		}
		return result;
	}

	static bool IsValidWord(std::string_view word);

	static int ComputeAverageRating(const std::vector<int>& ratings);

	bool IsStopWord(std::string_view word) const;

	std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

	QueryWord ParseQueryWord(std::string_view text) const;

	Query ParseQuery(std::string_view text) const;

	double ComputeWordInverseDocumentFreq(std::string_view word) const;

	std::vector<Document> GetMatchedWords(const std::map<int, double>& document_to_relevance) const;

	template <typename Predicate>
	std::vector<Document> FindAllDocuments([[maybe_unused]] std::execution::sequenced_policy, const Query& query, Predicate predicate) const {
		std::map<int, double> document_to_relevance;
		for (const std::string_view word : query.plus_words) {
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

		for (const std::string_view word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}

		return GetMatchedWords(document_to_relevance);
	}

	template <typename Predicate>
	std::vector<Document> FindAllDocuments(std::execution::parallel_policy policy, const Query& query, Predicate predicate) const {
		ConcurrentMap<int, double> document_to_relevance;
		std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
			[this, predicate, &document_to_relevance](const std::string_view word) {
				if (word_to_document_freqs_.count(word)) {
					const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
					for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
						const DocumentData& doc = documents_.at(document_id);
						if (predicate(document_id, doc.status, doc.rating)) {
							document_to_relevance[document_id].ref_to_value +=
								term_freq * inverse_document_freq;
						}
					}
				}
			}
		);

		std::for_each(policy, query.minus_words.begin(), query.minus_words.end(),
			[this, &document_to_relevance](const std::string_view word) {
				if (word_to_document_freqs_.count(word)) {
					for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
						document_to_relevance.Erase(document_id);
					}
				}
			}
		);

		return GetMatchedWords(document_to_relevance.BuildOrdinaryMap());
	}
};
