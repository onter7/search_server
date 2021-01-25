#pragma once

#include <set>
#include <string>
#include <vector>

#include "search_server.h"
#include "test_runner.h"

void TestConstructors();
void TestExcludeStopWordsFromAddedDocumentContent();
void TestAddDocuments();
void TestMinusWords();
void TestMatchDocuments();
void TestRelevanceCalculation();
void TestSorting();
void TestRatingCalculation();
void TestDocumentFiltering();
void TestSearchDocumentsWithStatus();

void TestSearchServer();
