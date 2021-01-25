#include "search_server.h"

SearchServer::SearchServer(const std::string& stop_words)
	: SearchServer(SplitIntoWords(stop_words))
{}

void SearchServer::AddDocument(int document_id, const std::string& document,
	DocumentStatus status, const std::vector<int>& ratings) {
	if (document_id < 0 || documents_.count(document_id)) {
		throw std::invalid_argument("Invalid document id: " + std::to_string(document_id));
	}
	const std::vector<std::string> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();
	for (const std::string& word : words) {
		if (!IsValidWord(word)) {
			throw std::invalid_argument("Invalid word in document: " + word);
		}
		word_to_document_freqs_[word][document_id] += inv_word_count;
	}
	documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
	document_ids_.push_back(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus doc_status) const {
	return FindTopDocuments(raw_query, [doc_status](int document_id, DocumentStatus status, int rating) {
		return status == doc_status;
	});
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

size_t SearchServer::GetDocumentCount() const { return documents_.size(); }

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
	int document_id) const {
	const Query query = ParseQuery(raw_query);
	std::vector<std::string> matched_words;
	for (const std::string& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			matched_words.push_back(word);
		}
	}
	for (const std::string& word : query.minus_words) {
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

int SearchServer::GetDocumentId(int index) const {
	return document_ids_.at(index);
}

bool SearchServer::IsValidWord(const std::string& word) {
	// A valid word must not contain special characters
	return none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
	});
}

bool SearchServer::IsStopWord(const std::string& word) const {
	return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
	std::vector<std::string> words;
	for (const std::string& word : SplitIntoWords(text)) {
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

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
	if (!IsValidWord(text)) {
		throw std::invalid_argument("Invalid query word: " + text);
	}
	bool is_minus = false;
	// Word shouldn't be empty
	if (text[0] == '-') {
		if (text.size() == 1 || (text.size() > 1 && text[1] == '-')) {
			throw std::invalid_argument("Invalid minus word: " + text);
		}
		is_minus = true;
		text = text.substr(1);
	}
	return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
	Query query;
	for (const std::string& word : SplitIntoWords(text)) {
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

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}