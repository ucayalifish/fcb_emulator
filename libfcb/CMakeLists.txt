cmake_minimum_required(VERSION 3.10)

project(fcb C)

set(LIB_HEADERS
        inc/fcb.h
        )

set(LIB_SOURCES
        src/utils.c
        )

add_library(fcb STATIC
        ${LIB_SOURCES}
        ${LIB_HEADERS}
        )

target_include_directories(fcb PUBLIC ${CMAKE_SOURCE_DIR}/libfcb/inc)

target_compile_options(fcb PRIVATE
        "-g3"
        "-Wall"
        "-Wextra"
        "--warn-no-unknown-pragmas"
        )
