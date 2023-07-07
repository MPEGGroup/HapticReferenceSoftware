project(iohaptics)


add_library(iohaptics src/IOJson.cpp include/IOJson.h src/IOJsonPrimitives.cpp include/IOJsonPrimitives.h src/IOBinary.cpp include/IOBinary.h src/IOBinaryPrimitives.cpp include/IOBinaryPrimitives.h src/IOBinaryBands.cpp include/IOBinaryBands.h include/IOBinaryFields.h include/IOStream.h src/IOStream.cpp include/IOConformance.h)
target_link_libraries(iohaptics PRIVATE types spiht)

if(BUILD_CATCH2)
    add_executable(test_iohaptics test/IOBinaryPrimitives.test.cpp test/IOBinaryBands.test.cpp test/IOBinary.test.cpp test/IOJson.test.cpp test/IOStream.test.cpp)

    target_link_libraries(test_iohaptics PRIVATE Catch2::Catch2WithMain iohaptics types)
    catch_discover_tests(test_iohaptics)
endif()
