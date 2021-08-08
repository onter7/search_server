﻿#pragma once

#include <vector>
#include <string_view>

#include "document.h"
#include "search_server.h"

void PrintDocument(const Document& document);

void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status);

void AddDocument(SearchServer& search_server, int document_id, const std::string_view document, DocumentStatus status,
	const std::vector<int>& ratings);

void FindTopDocuments(const SearchServer& search_server, const std::string_view raw_query);

void MatchDocuments(const SearchServer& search_server, const std::string_view query);