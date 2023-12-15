cmake_minimum_required(VERSION 3.7)

project(jsonc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g")

add_subdirectory(lib/json)
include_directories(lib/json)


add_executable(jsonc jsonc.cpp)
target_link_libraries(jsonc json)