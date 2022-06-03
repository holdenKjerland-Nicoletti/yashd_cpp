cmake_minimum_required(VERSION 3.22)
project(yash)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES include/Yash.h src/Yash.cpp include/utils.h src/utils.cpp src/Job.cpp include/Job.h src/Process.cpp include/Process.h)

add_library(yash SHARED
        ${SOURCES})
