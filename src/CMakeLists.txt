add_library(common
    "game.cpp" "game.h"
    "options.h" "options.cpp"
    "utility.h" "utility.cpp"
)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

target_link_libraries(common PRIVATE
    sfml-system sfml-graphics sfml-window
    imgui::imgui ImGui-SFML::ImGui-SFML
    cxxopts::cxxopts
)

add_executable("game_of_life" "main.cpp")
target_link_libraries("game_of_life" PRIVATE common sfml-system)
