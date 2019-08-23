cmake_minimum_required(VERSION 3.0)
project(ranges)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -pedantic -Werror -Wold-style-cast")

add_executable(range_test main.cpp student_test.cpp)
