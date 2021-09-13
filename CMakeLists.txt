cmake_minimum_required(VERSION 3.10)
project(fcb_emulator C)

set(CMAKE_C_STANDARD 11)

add_subdirectory("libnandemu")

add_subdirectory("libmembuf")

add_subdirectory("libfcb")

add_executable(fcb_emulator
        main.c
        utils.c
        experimental.c libfcb/inc/fcb.h libfcb/src/utils.c libfcb/src/utils.h)

target_link_libraries(fcb_emulator PUBLIC nandemu membuf fcb)

target_compile_options(fcb_emulator PRIVATE
        "-g3"
        "-Wall"
        "-Wextra"
        "-Wno-unknown-pragmas"
        )
