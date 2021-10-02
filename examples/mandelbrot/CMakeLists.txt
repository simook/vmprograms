cmake_minimum_required (VERSION 3.1.0)
project (mandelbrot CXX)

option(SSE4         "Override with SSE4.2" OFF)
option(AVX512F      "Enable AVX512-F" OFF)

set (SOURCES
	mandelbrot.cpp
	${CMAKE_SOURCE_DIR}/../lodepng/lodepng.cpp
)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -static -Ofast")
if (SSE4)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
elseif (AVX512F)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mavx512f -mavx512bw -mavx512cd -mavx512dq -mavx512vl -mavx512vnni")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

add_subdirectory(libsimdpp)

add_executable(mandelbrot ${SOURCES})
target_include_directories(mandelbrot PRIVATE ${CMAKE_SOURCE_DIR}/../lodepng)
target_include_directories(mandelbrot PRIVATE ${CMAKE_SOURCE_DIR}/libsimdpp)

if (SSE4)
	target_compile_definitions(mandelbrot PUBLIC SSE=1)
endif()
