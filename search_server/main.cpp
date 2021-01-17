#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

#include "search_server.h"
#include "test_runner.h"
#include "tests.h"

// ------------ ������ ������������� ----------------

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

    AddDocument(search_server, 1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, -1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, 3, "������� �� ����\x12��� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server, 4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    FindTopDocuments(search_server, "�������� -��"s);
    FindTopDocuments(search_server, "�������� --���"s);
    FindTopDocuments(search_server, "�������� -"s);

    MatchDocuments(search_server, "�������� ��"s);
    MatchDocuments(search_server, "������ -���"s);
    MatchDocuments(search_server, "������ --��"s);
    MatchDocuments(search_server, "�������� - �����"s);

	return 0;
}