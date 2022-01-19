project(Encoder)

add_executable(Encoder src/main.cpp src/PcmEncoder.cpp include/PcmEncoder.h src/AhapEncoder.cpp include/AhapEncoder.h src/IvsEncoder.cpp include/IvsEncoder.h src/WaveletEncoder.cpp include/WaveletEncoder.h)
target_link_libraries(Encoder PUBLIC tools types psychohapticModel filterbank)
target_link_libraries(Encoder PRIVATE pugixml::static iir::iir_static)

install(TARGETS Encoder DESTINATION bin)

if(BUILD_CATCH2)
    add_executable(test_Encoder test/PcmEncoder.test.cpp src/PcmEncoder.cpp test/AhapEncoder.test.cpp src/AhapEncoder.cpp test/IvsEncoder.test.cpp src/IvsEncoder.cpp src/WaveletEncoder.cpp)
    target_link_libraries(test_Encoder PUBLIC tools types filterbank psychohapticModel)
    target_link_libraries(test_Encoder PRIVATE Catch2::Catch2WithMain iir::iir_static pugixml::static)
    catch_discover_tests(test_Encoder)
endif()
