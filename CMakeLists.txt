# -*- mode: CMAKE; -*-

project(fuerte)
cmake_minimum_required(VERSION 2.8)

# need c++11
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(3rdParty/googletest)
add_subdirectory(3rdParty/curlpp2)

add_subdirectory(src)

add_subdirectory(tests)
add_subdirectory(examples)
