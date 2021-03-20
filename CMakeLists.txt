cmake_minimum_required(VERSION 3.0.0)
project(typeless VERSION 0.1.0)

include(CTest)
enable_testing()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(test)
add_subdirectory(sample)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)