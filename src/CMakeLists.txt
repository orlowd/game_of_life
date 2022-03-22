add_executable(game_of_life "main.cpp" "game.cpp" "game.h" "options.h" "options.cpp" "utility.h" "utility.cpp")

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(game_of_life PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(game_of_life PRIVATE cxxopts::cxxopts)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(game_of_life PRIVATE imgui::imgui)

find_package(ImGui-SFML CONFIG REQUIRED)
target_link_libraries(game_of_life PRIVATE ImGui-SFML::ImGui-SFML)
