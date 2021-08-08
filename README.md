# Search Server

A program for searching documents by search queries and ranking them.

## Description
* allows to set stop words, filter search results by "minus words" and document status
* [TF-IDF](https://en.wikipedia.org/wiki/Tf–idf) is used for ranking documents
* supports parallel processing of search queries
* does not store duplicate documents, for this purpose the duplicate deletion functionality was specially developed

## Build

The project supports building using CMake. External dependencies are not used, only the standard library.
