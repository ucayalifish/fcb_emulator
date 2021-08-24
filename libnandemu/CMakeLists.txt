cmake_minimum_required(VERSION 3.10)

project(nandemu C)

set(LIB_SOURCES
        src/nandemu.c
        src/block_state.c
        )

set(LIB_HEADERS
        inc/nandemu.h
        )

add_library(nandemu STATIC
        ${LIB_SOURCES}
        ${LIB_HEADERS}
        )

target_include_directories(nandemu PUBLIC ${CMAKE_SOURCE_DIR}/libnandemu/inc)

target_compile_options(nandemu PRIVATE "-g3")
