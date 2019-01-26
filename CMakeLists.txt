cmake_minimum_required(VERSION 3.12)
project(Function)

set(CMAKE_CXX_STANDARD 17)

add_executable(Function main.cpp function.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -D__GLIBCXX_DEBUG -Wall -Wextra")