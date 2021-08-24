cmake_minimum_required (VERSION 3.0.2)
project (qjs_app C)

set(CMAKE_C_FLAGS "-Wall -Wextra -std=gnu11 -O2 -g")

set(SOURCES
	main.c
	static_site.c
)
set_source_files_properties(main.c OBJECT_DEPENDS
	${CMAKE_SOURCE_DIR}/my.js
)

add_executable(jsapp ${SOURCES})
set_target_properties(jsapp PROPERTIES C_STANDARD 11)
if (NATIVE)
	target_compile_options(jsapp -march=native -Ofast -fno-fast-math)
endif()


## Unicode table generator executable ##
#add_executable(unicode_gen
#	"quickjs/cutils.c"
#	"quickjs/libunicode.c"
#	"quickjs/unicode_gen.c"
#)
#target_include_directories(unicode_gen PUBLIC "quickjs")
#add_custom_command(
#        OUTPUT libunicode-table.h
#        COMMAND unicode_gen unicode libunicode-table.h
#		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/quickjs
#        DEPENDS unicode_gen
#    )
#add_custom_target(gen_libunicode_table ALL
#        DEPENDS libunicode-table.h
#)


set (QJS_SOURCES
	quickjs/cutils.c
	quickjs/libregexp.c
	quickjs/qjs.c
	quickjs/quickjs.c
	quickjs/libbf.c
	quickjs/qjsc.c
	quickjs/quickjs-libc.c
	quickjs/libunicode.c
)
add_library(quickjs STATIC ${QJS_SOURCES})
set_target_properties(quickjs PROPERTIES C_STANDARD 11)
target_include_directories(quickjs PRIVATE "quickjs/")
target_compile_definitions(quickjs PUBLIC
	_GNU_SOURCE=1
	CONFIG_BIGNUM=1
	CONFIG_VERSION="2021-03-27"
)
target_compile_options(quickjs PRIVATE "-Wno-unused-parameter" "-Wno-sign-compare")
#add_dependencies(quickjs gen_libunicode_table)

target_link_libraries(jsapp -static quickjs pthread m)
