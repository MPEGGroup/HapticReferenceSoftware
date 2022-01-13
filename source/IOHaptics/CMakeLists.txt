project(iohaptics)

add_library(iohaptics src/IOJson.cpp include/IOJson.h src/IOBinary.cpp include/IOBinary.h src/IOBinaryPrimitives.cpp include/IOBinaryPrimitives.h)
target_link_libraries(iohaptics PRIVATE types nlohmann_json::nlohmann_json)
