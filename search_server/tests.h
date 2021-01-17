#pragma once

void TestConstructors() {
	{
		const vector<string> stop_words = { "in"s, "the"s };
		SearchServer server(stop_words);
		server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		ASSERT(server.FindTopDocuments("in"s).empty());
	}

	{
		const set<string> stop_words = { "in"s, "the"s };
		SearchServer server(stop_words);
		server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		ASSERT(server.FindTopDocuments("in"s).empty());
	}

	{
		SearchServer server("    in    the     "s);
		server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		ASSERT(server.FindTopDocuments("in"s).empty());
	}
}

void TestExcludeStopWordsFromAddedDocumentContent() {
	{
		SearchServer server({});
		server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& found_doc = found_docs[0];
		ASSERT_EQUAL(found_doc.id, 42);
	}

	{
		SearchServer server("in the"s);
		server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		ASSERT(server.FindTopDocuments("in"s).empty());
	}
}

void TestAddDocuments() {
	SearchServer server({});
	ASSERT_EQUAL(server.GetDocumentCount(), 0);
	server.AddDocument(1, "black white red yellow"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	ASSERT_EQUAL(server.GetDocumentCount(), 1);
	server.AddDocument(2, "black blue green purple"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	ASSERT_EQUAL(server.GetDocumentCount(), 2);
}

void TestMinusWords() {
	{
		SearchServer server({});
		server.AddDocument(1, "black white red yellow"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		server.AddDocument(2, "black blue green purple"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		const auto found_docs = server.FindTopDocuments("black -blue"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& found_doc = found_docs[0];
		ASSERT_EQUAL(found_doc.id, 1);
	}
}

void TestMatchDocuments() {
	SearchServer server({});
	server.AddDocument(1, "black white red green blue"s, DocumentStatus::ACTUAL, { 1, 2, 3 });

	{
		const auto [words, status] = server.MatchDocument("grass is green"s, 1);
		ASSERT_EQUAL(words.size(), 1);
		ASSERT_EQUAL(words[0], "green"s);
	}

	{
		const auto [words, status] = server.MatchDocument("black -white"s, 1);
		ASSERT_EQUAL(words.size(), 0);
	}
}

void TestRelevanceCalculation() {
	const double epsilon = 0.01;
	SearchServer server({});
	server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	const auto found_docs = server.FindTopDocuments("fluffy well-groomed cat"s);
	ASSERT(abs(found_docs[0].relevance - 0.65) < epsilon);
	ASSERT(abs(found_docs[1].relevance - 0.27) < epsilon);
	ASSERT(abs(found_docs[2].relevance - 0.08) < epsilon);
}

void TestSorting() {
	SearchServer server({});
	server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	const auto found_docs = server.FindTopDocuments("fluffy well-groomed cat"s);
	ASSERT_EQUAL(found_docs.size(), 3);
	ASSERT(found_docs[0].relevance >= found_docs[1].relevance);
	ASSERT(found_docs[1].relevance >= found_docs[2].relevance);
}

void TestRatingCalculation() {
	SearchServer server({});
	server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	const auto found_docs = server.FindTopDocuments("cat"s);
	ASSERT_EQUAL(found_docs.size(), 1);
	ASSERT_EQUAL(found_docs[0].rating, 2);
}

void TestDocumentFiltering() {
	SearchServer server({});
	server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
	server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::BANNED, { 2, 2, 2 });
	server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::IRRELEVANT, { 3, 3, 3 });

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
	SearchServer server({});
	server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
	server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::BANNED, { 2, 2, 2 });
	server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::IRRELEVANT, { 3, 3, 3 });

	const auto found_docs = server.FindTopDocuments("well-groomed cat"s, DocumentStatus::ACTUAL);
	ASSERT_EQUAL(found_docs.size(), 1);
	ASSERT_EQUAL(found_docs[0].id, 1);
}

void TestSearchServer() {
	RUN_TEST(TestConstructors);
	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
	RUN_TEST(TestAddDocuments);
	RUN_TEST(TestMinusWords);
	RUN_TEST(TestMatchDocuments);
	RUN_TEST(TestRelevanceCalculation);
	RUN_TEST(TestSorting);
	RUN_TEST(TestRatingCalculation);
	RUN_TEST(TestDocumentFiltering);
	RUN_TEST(TestSearchDocumentsWithStatus);
}
