#include "test_example_functions.h"

#include <iostream>
#include <stdexcept>

void PrintDocument(const Document& document) {
	using namespace std::literals;
	std::cout << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status) {
	using namespace std::literals;
	std::cout << "{ "s
		<< "document_id = "s << document_id << ", "s
		<< "status = "s << static_cast<int>(status) << ", "s
		<< "words ="s;
	for (const std::string_view word : words) {
		std::cout << ' ' << word;
	}
	std::cout << "}"s << std::endl;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string_view document, DocumentStatus status,
	const std::vector<int>& ratings) {
	using namespace std::literals;
	try {
		search_server.AddDocument(document_id, document, status, ratings);
	}
	catch (const std::exception& e) {
		std::cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << std::endl;
	}
}

void FindTopDocuments(const SearchServer& search_server, const std::string_view raw_query) {
	using namespace std::literals;
	std::cout << "Результаты поиска по запросу: "s << raw_query << std::endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query)) {
			PrintDocument(document);
		}
	}
	catch (const std::exception& e) {
		std::cout << "Ошибка поиска: "s << e.what() << std::endl;
	}
}

void MatchDocuments(const SearchServer& search_server, const std::string_view query) {
	using namespace std::literals;
	try {
		std::cout << "Матчинг документов по запросу: "s << query << std::endl;
		for (const int document_id : search_server) {
			const auto [words, status] = search_server.MatchDocument(query, document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	}
	catch (const std::exception& e) {
		std::cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << std::endl;
	}
}