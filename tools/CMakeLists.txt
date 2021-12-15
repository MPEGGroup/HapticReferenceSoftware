project(tools)

add_library(tools src/InputParser.cpp src/WavParser.cpp)
add_library(dr_wav INTERFACE)
target_include_directories(dr_wav INTERFACE ${CMAKE_BINARY_DIR}/_deps/dr_libs-src/)
target_link_libraries(tools dr_wav)

add_executable(test_tools test/InputParser.test.cpp test/WavParser.test.cpp)
target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools)
#target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools dr_libs)
catch_discover_tests(test_tools)
