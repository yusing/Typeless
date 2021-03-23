cmake_minimum_required(VERSION 3.0.0)
project(typeless_test)
set(CMAKE_CXX_STANDARD 14)

# Setup testing
add_subdirectory(googletest)
enable_testing()
include_directories(../include)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(internal)
add_definitions(-D__TYPELESS_TEST)
add_executable(typeless_test test.cpp object_test.h array_test.h)

target_link_libraries(typeless_test gtest gtest_main)
add_test(typeless_test typeless_test)