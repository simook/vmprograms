cmake_minimum_required (VERSION 3.0.2)
project (qjs_app C)

option(NATIVE   "Enable native performance" OFF)

set(CMAKE_C_FLAGS "-Wall -Wextra -std=gnu11 -O2 -g")

set(SOURCES
	src/main.c
	src/http.c
	static_site.c
)
set_source_files_properties(src/main.c OBJECT_DEPENDS
	${CMAKE_SOURCE_DIR}/src/my.js
)

add_executable(jsapp ${SOURCES})
set_target_properties(jsapp PROPERTIES C_STANDARD 11)
if (NATIVE)
	target_compile_options(jsapp -march=native -Ofast -fno-fast-math)
endif()


set (QJS_SOURCES
	quickjs/cutils.c
	quickjs/libregexp.c
	quickjs/quickjs.c
	quickjs/libbf.c
	quickjs/quickjs-libc.c
	quickjs/libunicode.c
	quickjs/tests/bjson.c
)
add_library(quickjs STATIC ${QJS_SOURCES})
set_target_properties(quickjs PROPERTIES C_STANDARD 11)
target_include_directories(quickjs PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_definitions(quickjs PUBLIC
	_GNU_SOURCE=1
	CONFIG_BIGNUM=1
	CONFIG_VERSION="2021-03-27"
)
target_compile_options(quickjs PRIVATE "-Wno-unused-parameter" "-Wno-sign-compare")

target_link_libraries(jsapp -static quickjs pthread m)
