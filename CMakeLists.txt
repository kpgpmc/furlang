cmake_minimum_required(VERSION 3.14)

project(furlang VERSION 0.0.0 LANGUAGES CXX)

find_package(GTest REQUIRED)

add_subdirectory(furlang)
add_subdirectory(furc)