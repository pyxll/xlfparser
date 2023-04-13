cmake_minimum_required(VERSION 3.8)
project(xlfparser_tests C CXX)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

include_directories(include)

add_executable(tests tests/main.cpp tests/tests.cpp)
add_executable(example example.cpp)

enable_testing()
add_test(tests tests)
