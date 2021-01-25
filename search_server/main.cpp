#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "test_runner.h"
#include "tests.h"

using namespace std;

void PrintDocument(const Document& document) {
	cout << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
	cout << "{ "s
		<< "document_id = "s << document_id << ", "s
		<< "status = "s << static_cast<int>(status) << ", "s
		<< "words ="s;
	for (const string& word : words) {
		cout << ' ' << word;
	}
	cout << "}"s << endl;
}

void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status,
	const vector<int>& ratings) {
	try {
		search_server.AddDocument(document_id, document, status, ratings);
	}
	catch (const exception& e) {
		cout << "������ ���������� ��������� "s << document_id << ": "s << e.what() << endl;
	}
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
	cout << "���������� ������ �� �������: "s << raw_query << endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query)) {
			PrintDocument(document);
		}
	}
	catch (const exception& e) {
		cout << "������ ������: "s << e.what() << endl;
	}
}

void MatchDocuments(const SearchServer& search_server, const string& query) {
	try {
		cout << "������� ���������� �� �������: "s << query << endl;
		const int document_count = search_server.GetDocumentCount();
		for (int index = 0; index < document_count; ++index) {
			const int document_id = search_server.GetDocumentId(index);
			const auto [words, status] = search_server.MatchDocument(query, document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	}
	catch (const exception& e) {
		cout << "������ �������� ���������� �� ������ "s << query << ": "s << e.what() << endl;
	}
}

int main() {
	TestSearchServer();
	cout << "Search server testing finished"s << endl;

	SearchServer search_server("� � ��"s);
	RequestQueue request_queue(search_server);

	search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "������� ��� ������ ������� "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

	
	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("������ ������"s);
	}
	
	request_queue.AddFindRequest("�������� ��"s);	
	request_queue.AddFindRequest("������� �������"s);	
	request_queue.AddFindRequest("�������"s);
	cout << "��������, �� ������� ������ �� ������� "s << request_queue.GetNoResultRequests();

	return 0;
}