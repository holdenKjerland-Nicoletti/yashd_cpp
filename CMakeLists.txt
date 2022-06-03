cmake_minimum_required(VERSION 3.22)
project(yashd_cpp)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(yash)

add_executable(yash_exec yash/main.cpp)

target_link_libraries(yash_exec yash)