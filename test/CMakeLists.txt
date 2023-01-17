add_executable(test_gol
    "test_grid.cpp" "test_options.cpp"
)

find_package(Catch2 CONFIG REQUIRED)

target_link_libraries(test_gol PRIVATE
    Catch2::Catch2WithMain
    common
)

include(CTest)
include(Catch)
catch_discover_tests(test_gol)
