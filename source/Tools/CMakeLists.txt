project(tools)

add_library(tools src/InputParser.cpp src/WavParser.cpp src/PsychohapticModel.cpp)

add_executable(test_tools test/InputParser.test.cpp test/WavParser.test.cpp test/PsychohapticModel.test.cpp)
target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools)
catch_discover_tests(test_tools)
