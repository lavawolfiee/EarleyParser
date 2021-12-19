cmake_minimum_required(VERSION 3.17)
project(Earley)

set(CMAKE_CXX_STANDARD 17)

add_executable(Earley main.cpp EarleyParser.h)
add_executable(Tests tests.cpp EarleyParser.h)