cmake_minimum_required (VERSION 3.21)

add_executable(game_of_life "main.cpp")

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(game_of_life PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(game_of_life PRIVATE cxxopts::cxxopts)
