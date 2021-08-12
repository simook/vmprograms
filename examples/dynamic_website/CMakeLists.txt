cmake_minimum_required (VERSION 3.0.2)
project (dynamic_website CXX)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -O2 -g")

set(SOURCES
	main.cpp
)

add_subdirectory(json)

add_executable(dynsite ${SOURCES})
set_target_properties(dynsite PROPERTIES CXX_STANDARD 17)
if (NATIVE)
	target_compile_options(dynsite -march=native -Ofast -fno-fast-math)
endif()
target_link_libraries(dynsite -static nlohmann_json)
