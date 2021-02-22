#include <iostream>
#include <deque>

#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
	std::map<std::vector<std::string>, std::set<int>> words_to_document_ids;    
	for (const int document_id : search_server) {
		const auto& word_to_freq = search_server.GetWordFrequencies(document_id);
		std::vector<std::string> words;
        words.reserve(word_to_freq.size());
		for (const auto& [word, freq] : word_to_freq) {
			words.emplace_back(word);
		}
		words_to_document_ids[words].insert(document_id);
	}
	for (auto& [words, document_ids] : words_to_document_ids) {
		if (document_ids.size() > 1) {			
			for (auto it = std::next(document_ids.begin()); it != document_ids.end(); ++it) {
				search_server.RemoveDocument(*it);
                std::cout << "Found duplicate document id "s << *it << std::endl;
			}
		}
	}
}

void AddDocument(SearchServer& search_server,int document_id, const std::string& document,
    DocumentStatus status, const std::vector<int>& ratings) {
    search_server.AddDocument(document_id, document, status, ratings);
}

int main() {
    SearchServer search_server("and with"s);

    AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // дубликат документа 2, будет удалён
    AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // отличие только в стоп-словах, считаем дубликатом
    AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // множество слов такое же, считаем дубликатом документа 1
    AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // добавились новые слова, дубликатом не является
    AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
    AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

    // есть не все слова, не является дубликатом
    AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // слова из разных документов, не является дубликатом
    AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
    RemoveDuplicates(search_server);
    cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;

	return 0;
}