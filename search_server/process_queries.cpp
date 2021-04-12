#include <algorithm>
#include <execution>
#include <string_view>

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
	const std::vector<Document> result = std::reduce(
		std::execution::par,
		queries_results.begin(),
		queries_results.end(),
		std::vector<Document>{},
		[](auto result, const auto& documents) {
			result.insert(result.end(), documents.begin(), documents.end());
			return result;
		}
	);
	return result;
}