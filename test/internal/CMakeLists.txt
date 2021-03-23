cmake_minimum_required(VERSION 3.0.0)
project(typeless_internal_test)

# Setup testing
enable_testing()
add_executable(typeless_internal_test internal_test.cpp)

target_link_libraries(typeless_internal_test gtest gtest_main)
add_test(typeless_internal_test typeless_internal_test)