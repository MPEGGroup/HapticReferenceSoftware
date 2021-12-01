project(tools)

add_library(tools src/InputParser.cpp src/WavParser.cpp)

add_executable(test_tools src/InputParser.test.cpp src/WavParser.test.cpp)
target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools)
catch_discover_tests(test_tools)