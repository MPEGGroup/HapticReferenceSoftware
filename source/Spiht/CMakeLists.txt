project(spiht)

add_library(spiht include/Spiht_Enc.h src/Spiht_Enc.cpp include/Spiht_Dec.h src/Spiht_Dec.cpp include/ArithEnc.h src/ArithEnc.cpp include/ArithDec.h src/ArithDec.cpp)

add_executable(test_spiht test/Spiht_Enc.test.cpp test/ArithEnc.test.cpp)
target_link_libraries(test_spiht PRIVATE Catch2::Catch2WithMain spiht)

catch_discover_tests(test_spiht)
