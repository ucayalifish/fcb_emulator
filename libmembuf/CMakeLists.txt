cmake_minimum_required(VERSION 3.10)

project(membuf C)

set(LIB_SOURCES
        membuf.c
        )

set(LIB_HEADERS
        membuf.h
        )

add_library(membuf STATIC
        ${LIB_SOURCES}
        ${LIB_HEADERS}
        )

target_include_directories(membuf PUBLIC ${CMAKE_SOURCE_DIR}/libmembuf)

target_link_libraries(membuf PUBLIC nandemu)

target_compile_options(membuf PRIVATE "-g3")
