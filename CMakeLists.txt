cmake_minimum_required(VERSION 3.10)
project(kasm_toolchain C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SRC_FILES "src/*.c" "src/*.h")
add_library(kasm_shared SHARED
    ${SRC_FILES}
)

set_target_properties(kasm_shared PROPERTIES OUTPUT_NAME "kasm")

target_include_directories(kasm_shared PUBLIC src)

if (WIN32)
    set(GETOPT_SRC cli/vendor/getopt.c)
endif()

add_executable(kasm
    cli/main.c
    ${GETOPT_SRC}
)

target_link_libraries(kasm kasm_shared)

# Build KM8 target :)
add_library(km8 SHARED
    targets/km8/km8.c
)

target_include_directories(km8 PRIVATE targets/km8 src)

set_target_properties(km8 PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/targets 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/targets 
    OUTPUT_NAME "km8"
)
