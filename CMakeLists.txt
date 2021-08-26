cmake_minimum_required(VERSION 3.10)
project(fcb_emulator C)

set(CMAKE_C_STANDARD 11)

add_subdirectory("libnandemu")

add_subdirectory("libmembuf")

add_executable(fcb_emulator
        main.c
        utils.c
        experimental.c)

target_link_libraries(fcb_emulator PUBLIC nandemu membuf)

target_compile_options(fcb_emulator PRIVATE
        "-g3"
        "-Wall"
        "-Wextra"
        )
