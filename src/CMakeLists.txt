set(SRCS
	main.cpp
	document.cpp
	process_queries.cpp
	read_input_functions.cpp
	remove_duplicates.cpp
	request_queue.cpp
	search_server.cpp
	string_processing.cpp
	test_example_functions.cpp
)

set(HDRS
	concurrent_map.h
	document.h
	log_duration.h
	paginator.h
	process_queries.h
	read_input_functions.h
	remove_duplicates.h
	request_queue.h
	search_server.h
	string_processing.h
	test_example_functions.h
)

add_executable(search_server ${SRCS} ${HDRS})