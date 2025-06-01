cmake_minimum_required(VERSION 3.7)

project(ChimpGB)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)

add_subdirectory(src)
