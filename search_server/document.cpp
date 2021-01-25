#include "document.h"

Document::Document()
	: id(0)
	, relevance(0.0)
	, rating(0)
{}

Document::Document(int id, double relevance, int rating)
	: id(id)
	, relevance(relevance)
	, rating(rating)
{}

std::ostream& operator<<(std::ostream& os, const Document& document) {
	return os << "{ document_id = " << document.id
		<< ", relevance = " << document.relevance
		<< ", rating = " << document.rating << " }";
}