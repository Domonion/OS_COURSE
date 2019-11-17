cmake_minimum_required(VERSION 3.0)
project(kidshell)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_GLAGS} -Wall -Wextra -Wshadow -pedantic -fsanitize=address,undefined -D_GLIBCXX_DEBUG")

add_executable(kidshell main.cpp)