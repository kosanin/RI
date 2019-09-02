cmake_minimum_required(VERSION 3.14)
project(RI)

find_package(Threads)

set(CMAKE_CXX_STANDARD 17)

add_executable(RI main.cpp individual.cpp individual.h GA.cpp GA.h)