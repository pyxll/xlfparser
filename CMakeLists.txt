cmake_minimum_required(VERSION 3.10)
project(xlfparser_tests C CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
include_directories(include)

add_executable(tests tests/main.cpp tests/tests.cpp)
add_executable(example example.cpp)

enable_testing()
add_test(xlfparser_tests xlfparser_tests)
