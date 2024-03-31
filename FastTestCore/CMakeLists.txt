cmake_minimum_required(VERSION 3.10)

project(FastTestCore)

file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/FastTestCore/include/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/FastTestCore/src/*.cpp")

# print all files
add_library(FastTestCore SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_precompile_headers(FastTestCore PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/standard/pch.hpp")

target_link_libraries(
    FastTestCore
    absl::base
    absl::flat_hash_map
    Microsoft.GSL::GSL
)

target_compile_definitions(FastTestCore PRIVATE FASTTEST_EXPORTS)
