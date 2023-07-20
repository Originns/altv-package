cmake_minimum_required(VERSION 3.10)

project(extract_package)

set(CMAKE_CXX_STANDARD 20)

add_executable(extract_package main.cpp)
target_link_libraries(extract_package alt-package)
