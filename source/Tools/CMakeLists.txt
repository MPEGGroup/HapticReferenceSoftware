project(tools)

add_library(tools src/InputParser.cpp include/InputParser.h src/WavParser.cpp include/WavParser.h src/OHMData.cpp include/OHMData.h src/Tools.cpp include/Tools.h)

if(BUILD_CATCH2)
    add_executable(test_tools test/InputParser.test.cpp test/WavParser.test.cpp test/OHMData.test.cpp)
    target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools)
    catch_discover_tests(test_tools)
endif()