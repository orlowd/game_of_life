add_executable(test_gol "test_main.cpp" "test_grid.cpp")
target_link_libraries(test_gol PRIVATE common)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(test_gol PRIVATE Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(test_gol)
