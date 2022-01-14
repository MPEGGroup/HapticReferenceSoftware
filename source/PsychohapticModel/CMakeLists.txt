project(psychohapticModel)

add_library(psychohapticModel include/PsychohapticModel.h src/PsychohapticModel.cpp)

add_executable(test_psychohapticModel test/PsychohapticModel.test.cpp src/PsychohapticModel.cpp)
target_link_libraries(test_psychohapticModel PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(test_psychohapticModel)
