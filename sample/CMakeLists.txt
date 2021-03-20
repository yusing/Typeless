cmake_minimum_required(VERSION 3.0.0)
project(typeless_sample)
set(CMAKE_CXX_STANDARD 14)

include_directories(../include)
add_executable(typeless_sample sample.cpp)