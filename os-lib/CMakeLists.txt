cmake_minimum_required(VERSION 3.0)
project(lib)

add_library(_static STATIC static.cpp)

add_library(_dynamic_linked SHARED dynamic_linked.cpp)

add_executable(lib main.cpp)

target_link_libraries(lib _static)

target_link_libraries(lib _dynamic_linked)

target_link_libraries(lib -ldl)

add_library(_dynamic_not_linked SHARED _dynamic_not_linked.cpp)