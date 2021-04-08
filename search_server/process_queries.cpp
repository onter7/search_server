#include <algorithm>
#include <execution>
#include <string_view>

#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
	const SearchServer& search_server,
	const std::vector<std::string>& queries) {
	std::vector<std::vector<Document>> result(queries.size());
	std::transform(
		std::execution::par,
		queries.begin(),
		queries.end(),
		result.begin(),
		[&search_server](std::string_view query) {
			return search_server.FindTopDocuments(query);
		}
	);
	return result;
}

std::vector<Document> ProcessQueriesJoined(
	const SearchServer& search_server,
	const std::vector<std::string>& queries) {
	auto query_results = ProcessQueries(search_server, queries);
	std::vector<Document> result = std::reduce(
		std::execution::par,
		query_results.begin(),
		query_results.end(),
		std::vector<Document>{},
		[](auto result, const auto documents) {
			result.insert(result.end(), documents.begin(), documents.end());
			return result;
		}
	);
	return result;
}