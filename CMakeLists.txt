cmake_minimum_required(VERSION 3.0.0)

project(XTaskTests)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS}  -Wall -O2 -std=c++14")

include_directories(.)

add_executable(test-basic tests/test-basic.cpp)