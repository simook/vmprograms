cmake_minimum_required (VERSION 3.1.0)
project (jsonmin CXX)

option(MULTIPROCESS "Enable multi-processing" OFF)

set (SOURCES
	minifier.cpp
)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=gnu++17 -static -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

add_subdirectory(simdjson)

add_executable(jsonmin ${SOURCES})
target_link_libraries(jsonmin PRIVATE simdjson)

if (MULTIPROCESS)
	target_compile_definitions(jsonmin PUBLIC MULTIPROCESS=1)
endif()
