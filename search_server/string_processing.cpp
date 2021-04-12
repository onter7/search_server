#include <cstddef>

#include "string_processing.h"

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
	std::vector<std::string_view> result;
	size_t pos = 0;
	const size_t pos_end = text.npos;
	while (true) {
		const size_t space = text.find(' ', pos);
		if (space == pos_end) {
			result.push_back(text.substr(pos));
			break;
		}
		else {
			result.push_back(text.substr(pos, space - pos));
			pos = space + 1;
		}
	}

	return result;
}