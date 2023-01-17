cmake_minimum_required(VERSION 3.21)

project("game_of_life" CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("src")
add_subdirectory("test")
