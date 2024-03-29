cmake_minimum_required(VERSION 3.10)

project(FastTest)

if(WIN32)
		add_definitions(-DFASTTEST_EXPORTS)
endif(WIN32)


set(CMAKE_CXX_STANDARD 17)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/FastTestCore/include)

add_subdirectory(FastTestCore)

add_subdirectory(FastTests)
