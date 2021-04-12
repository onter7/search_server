#include <iostream>

#include "string_processing.h"
#include "search_server.h"

const std::map<std::string_view, double> empty_map = {};

SearchServer::SearchServer(std::string_view stop_words)
	: SearchServer(SplitIntoWords(stop_words))
{}

SearchServer::SearchServer(const std::string& stop_words)
	: SearchServer(std::string_view(stop_words.c_str(), stop_words.size()))
{}

void SearchServer::AddDocument(int document_id, std::string_view document,
	DocumentStatus status, const std::vector<int>& ratings) {
	if (document_id < 0 || documents_.count(document_id)) {
		throw std::invalid_argument("Invalid document id: " + std::to_string(document_id));
	}
	const std::vector<std::string_view> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();
	std::map<std::string_view, double> word_to_freq;
	for (const std::string_view word : words) {
		if (!IsValidWord(word)) {
			throw std::invalid_argument("Invalid word in document: " + std::string(word));
		}
	}
	DocumentData document_data;
	for (const std::string_view word : words) {
		const auto [it, success] = document_data.words.insert(std::string(word));
		const std::string_view inserted_word(it->c_str(), it->size());
		word_to_document_freqs_[inserted_word][document_id] += inv_word_count;
		document_data.word_to_freq[inserted_word] += inv_word_count;
	}
	document_data.rating = ComputeAverageRating(ratings);
	document_data.status = status;
	documents_.emplace(document_id, std::move(document_data));
	document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus doc_status) const {
	return FindTopDocuments(raw_query, [doc_status](int document_id, DocumentStatus status, int rating) {
		return status == doc_status;
	});
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

size_t SearchServer::GetDocumentCount() const { return documents_.size(); }

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
	int document_id) const {
	return MatchDocument(std::execution::seq, raw_query, document_id);
}

bool SearchServer::IsValidWord(std::string_view word) {
	// A valid word must not contain special characters
	return std::none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
	});
}

bool SearchServer::IsStopWord(std::string_view word) const {
	return stop_words_.count(word) > 0;
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
	std::vector<std::string_view> words;
	for (const std::string_view word : SplitIntoWords(text)) {
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}
	return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
	int rating_sum = 0;
	for (const int rating : ratings) {
		rating_sum += rating;
	}
	return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
	if (!IsValidWord(text)) {
		throw std::invalid_argument("Invalid query word: " + std::string(text));
	}
	bool is_minus = false;
	// Word shouldn't be empty
	if (text[0] == '-') {
		if (text.size() == 1 || (text.size() > 1 && text[1] == '-')) {
			throw std::invalid_argument("Invalid minus word: " + std::string(text));
		}
		is_minus = true;
		text = text.substr(1);
	}
	return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
	Query query;
	for (const std::string_view word : SplitIntoWords(text)) {
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

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const {
	return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

std::set<int>::const_iterator SearchServer::begin() const { return document_ids_.begin(); }

std::set<int>::const_iterator SearchServer::end() const { return document_ids_.end(); }

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
	if (documents_.count(document_id) == 0) {
		return empty_map;
	}
	return documents_.at(document_id).word_to_freq;
}

void SearchServer::RemoveDocument(int document_id) {
	const auto it = documents_.find(document_id);
	if (it == documents_.end()) return;

	for (const auto& [word, freq] : it->second.word_to_freq) {
		word_to_document_freqs_.at(word).erase(document_id);
	}
	documents_.erase(document_id);
	document_ids_.erase(document_id);
}