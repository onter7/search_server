cmake_minimum_required(VERSION 3.10)

project(SearchServer CXX)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_subdirectory(src)