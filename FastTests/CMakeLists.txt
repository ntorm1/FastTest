cmake_minimum_required(VERSION 3.14)
project(FastTests)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  FastTests
  
 "src/test_exchange.cpp" "include/pch.hpp")
target_link_libraries(
  FastTests
  GTest::gtest_main
  FastTestCore
)

target_include_directories(FastTests PUBLIC ${CMAKE_SOURCE_DIR}/FastTestCore/include)

include(GoogleTest)
gtest_discover_tests(FastTests)

include_directories(include)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp")

