project(iohaptics)

add_library(iohaptics src/IOJson.cpp include/IOJson.h src/IOBinary.cpp include/IOBinary.h src/IOBinaryPrimitives.cpp include/IOBinaryPrimitives.h src/IOBinaryBands.cpp include/IOBinaryBands.h "include/IOStream.h" "src/IOStream.cpp"  "test/IOStream.test.cpp")
target_link_libraries(iohaptics PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(iohaptics PRIVATE types spiht)

if(BUILD_CATCH2)
    add_executable(test_iohaptics test/IOBinaryPrimitives.test.cpp test/IOBinaryBands.test.cpp test/IOBinary.test.cpp test/IOJson.test.cpp "include/IOStream.h" "src/IOStream.cpp"  "test/IOStream.test.cpp")
    target_link_libraries(test_iohaptics PRIVATE Catch2::Catch2WithMain iohaptics nlohmann_json::nlohmann_json types)
    catch_discover_tests(test_iohaptics)
endif()
