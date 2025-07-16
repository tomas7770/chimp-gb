cmake_minimum_required(VERSION 3.7)

project(ChimpGB)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
find_package(nlohmann_json 3.11.0 REQUIRED)

add_subdirectory(src)
