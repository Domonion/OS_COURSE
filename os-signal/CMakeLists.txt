cmake_minimum_required(VERSION 3.0)
project(find)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -pedantic -D_GLIBCXX_DEBUG")

include_directories(${PROJECT_SOURCE_DIR})
add_executable(signal main.cpp utils.cpp utils.hpp)
