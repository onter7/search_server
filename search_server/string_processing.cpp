#include <cstddef>

#include "string_processing.h"

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
	std::vector<std::string_view> result;
	while (true) {
		const std::size_t space = text.find(' ');
		if (space == text.npos) {
			result.push_back(text.substr());
			break;
		}
		else {
			result.push_back(text.substr(0, space));
			text.remove_prefix(space + 1);
		}
	}

	return result;
}