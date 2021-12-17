project(RM0_Encoder)

add_executable(RM0_Encoder src/main.cpp include/IOJson.h src/IOJson.cpp)
target_link_libraries(RM0_Encoder tools)
