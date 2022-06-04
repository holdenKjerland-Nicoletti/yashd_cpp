cmake_minimum_required(VERSION 3.22)
project(yashd_cpp)

#set(CMAKE_CXX_COMPILER /usr/local/Cellar/gcc/11.3.0_1/bin/g++-11)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(yash)
add_subdirectory(yashd)

# https://cprieto.com/posts/2021/03/cmake-and-threads.html
find_package(Threads REQUIRED)

add_executable(yash_exec yash/main.cpp)
target_link_libraries(yash_exec yash)

add_executable(yashd_exec yashd/main.cpp yashd/main.cpp)
target_link_libraries(yashd_exec yashd)
target_link_libraries(yashd_exec Threads::Threads)