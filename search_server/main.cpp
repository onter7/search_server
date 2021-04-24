﻿#include "process_queries.h"
#include "search_server.h"
#include "test_example_functions.h"

#include <execution>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
	SearchServer search_server("and with"s);

	const vector<string> queries = {
		"white cat and yellow hat"s,
		"curly cat curly tail"s,
		"nasty dog with big eyes"s,
		"nasty pigeon john"s
	};

	for (size_t i = 0; i < queries.size(); ++i) {
		search_server.AddDocument(i + 1, queries[i], DocumentStatus::ACTUAL, { 1, 2 });
	}

	cout << "ACTUAL by default:"s << endl;
	// последовательная версия
	for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
		PrintDocument(document);
	}
	cout << "BANNED:"s << endl;
	// последовательная версия
	for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
		PrintDocument(document);
	}

	cout << "Even ids:"s << endl;
	// параллельная версия
	for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
		PrintDocument(document);
	}

	return 0;
}