cmake_minimum_required(VERSION 3.22)
project(yash)

#set(CMAKE_CPP_COMPILER /usr/local/Cellar/gcc/11.3.0_1/bin/g++-11)
set(CMAKE_CXX_STANDARD 20)


set(SOURCES include/Yash.h src/Yash.cpp include/utils.h src/utils.cpp src/Job.cpp include/Job.h src/Process.cpp include/Process.h)

add_library(yash SHARED
        ${SOURCES})


