cmake_minimum_required (VERSION 3.1.0)
project (mandelbrot CXX)

option(NATIVE       "Enable native performance" ON)
option(AVX512F      "Enable AVX512-F" OFF)

set (SOURCES
	mandelbrot.cpp
	${CMAKE_SOURCE_DIR}/../lodepng/lodepng.cpp
)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -march=native -static -O2 -g")
if (AVX512F)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")
endif()

add_subdirectory(libsimdpp)

add_executable(mandelbrot ${SOURCES})
target_include_directories(mandelbrot PRIVATE ${CMAKE_SOURCE_DIR}/../lodepng)
target_include_directories(mandelbrot PRIVATE ${CMAKE_SOURCE_DIR}/libsimdpp)
target_compile_definitions(mandelbrot PUBLIC
#	SIMDPP_64_BITS=1
#	SIMDPP_X86=1
#	SIMDPP_USE_AVX2=1
)
target_compile_options(mandelbrot PUBLIC "-Ofast -march=native")
