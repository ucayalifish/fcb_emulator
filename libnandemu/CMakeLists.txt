cmake_minimum_required(VERSION 3.20)

project(nandemu C)

set(LIB_SOURCES
        nandemu_api.c
        )

set(LIB_HEADERS
        nandemu.h
        )

add_library(nandemu STATIC
        ${LIB_SOURCES}
        ${LIB_HEADERS}
        )

message(STATUS "Message: " ${CMAKE_SOURCE_DIR}/libnandemu)

target_include_directories(nandemu PUBLIC ${CMAKE_SOURCE_DIR}/libnandemu)
