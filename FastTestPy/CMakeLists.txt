cmake_minimum_required(VERSION 3.18)

project(fasttest_internal)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python_EXECUTABLE: ${Python_EXECUTABLE}")
message(STATUS "Python_INCLUDE_DIRS: ${Python_INCLUDE_DIRS}")
message(STATUS "Python_LIBRARIES: ${Python_LIBRARIES}")

find_package(pybind11 CONFIG)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	${Python_INCLUDE_DIRS}
	${pybind11_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")

pybind11_add_module(fasttest_internal SHARED ${SOURCE_FILES})

target_precompile_headers(fasttest_internal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp")

target_link_directories(fasttest_internal
	PRIVATE
	${CMAKE_BINARY_DIR}/lib
)

target_link_libraries(fasttest_internal
	PUBLIC
	FastTestCore
)

