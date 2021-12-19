cmake_minimum_required(VERSION 3.17)
project(Earley)

set(CMAKE_CXX_STANDARD 17)

add_executable(Earley early_in_one_file.cpp)
add_executable(Tests tests.cpp)