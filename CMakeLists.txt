cmake_minimum_required(VERSION 3.20)
project(fcb_emulator C)

set(CMAKE_C_STANDARD 11)

add_subdirectory("libnandemu")

add_executable(fcb_emulator main.c)

target_link_libraries(fcb_emulator PUBLIC nandemu)
