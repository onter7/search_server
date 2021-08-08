# Search Server

A program for searching documents by search queries and ranking them. 
The search server allows to set stop words, as well as filter search results by "minus words" and document status.
[TF-IDF](https://en.wikipedia.org/wiki/Tf–idf) is used for ranking documents.
The search server supports parallel processing of search queries.

The project does not use external dependencies, only the standard library. 

## Build

The project supports building using CMake.
