cmake_minimum_required(VERSION 3.0)
project(find)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -pedantic -fsanitize=address,undefined -D_GLIBCXX_DEBUG")

add_executable(find main.cpp)
