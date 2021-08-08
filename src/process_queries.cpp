#include <algorithm>
#include <cstddef>
#include <execution>

#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
	const SearchServer& search_server,
	const std::vector<std::string>& queries) {
	std::vector<std::vector<Document>> queries_results(queries.size());
	std::transform(
		std::execution::par,
		queries.begin(),
		queries.end(),
		queries_results.begin(),
		[&search_server](const std::string& query) {
			return search_server.FindTopDocuments(query);
		}
	);
	return queries_results;
}

std::vector<Document> ProcessQueriesJoined(
	const SearchServer& search_server,
	const std::vector<std::string>& queries) {
	const auto queries_results = ProcessQueries(search_server, queries);
	const std::size_t result_size = std::transform_reduce(
		std::execution::par,
		queries_results.begin(),
		queries_results.end(),
		std::size_t(0),
		std::plus<>{},
		[](const std::vector<Document>& documents) {
			return documents.size();
		}
	);
	std::vector<Document> result;
	result.reserve(result_size);
	for (const auto& documents : queries_results) {
		result.insert(result.end(), documents.begin(), documents.end());
	}
	return result;
}