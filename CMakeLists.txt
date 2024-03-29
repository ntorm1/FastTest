cmake_minimum_required(VERSION 2.8.6)
project (FastTest)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

#compile all *.cpp source files under src folder
file (GLOB_RECURSE SOURCES "src/*.cpp")
		
#output library export file *.lib and
#output macro definitions include file
include (GenerateExportHeader)

add_library(FastTest SHARED ${SOURCES})
GENERATE_EXPORT_HEADER (FastTest
    BASE_NAME FastTest
    EXPORT_MACRO_NAME FastTest_EXPORT
    EXPORT_FILE_NAME FastTest_EXPORT.h
    STATIC_DEFINE FastTest_BUILT_AS_STATIC
)

target_include_directories(FastTest
 PUBLIC
 ${CMAKE_SOURCE_DIR}/include
 ${CMAKE_SOURCE_DIR}/include/Asset
 ${CMAKE_SOURCE_DIR}/include/rapidjson
 ${CMAKE_SOURCE_DIR}/include/rapidjson/error
 ${CMAKE_SOURCE_DIR}/include/rapidjson/internal
 )

IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Release ... FORCE )
   message("RELEASE BUILD...")
ENDIF()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" CXX_HAS_STD_20)
if(CXX_HAS_STD_20)
    set(CMAKE_CXX_FLAGS_RELEASE "-std=c++20")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-std=c++2a")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

