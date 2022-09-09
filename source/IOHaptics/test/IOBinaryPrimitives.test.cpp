/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2021, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using haptics::io::IOBinaryPrimitives;

const std::string filename = "testing_IOBinaryPrimitives.bin";
constexpr float precision_threshold = 0.001;

TEST_CASE("haptics::types::IOBinaryPrimitives on strings") {
  const std::vector<const char *> testingSet = {
      "Hello Haptic World !", "placeholder", "42",
      "this will be the future haptic description for your awsome MPEG file"};

  for (const char *testingValue : testingSet) {
    const std::string testingString(testingValue);
    DYNAMIC_SECTION("Write string (TESTING CASE: " + testingString + ")") {
      std::ofstream file(filename, std::ios::out | std::ios::binary);
      REQUIRE(file);
      std::vector<bool> bitset;
      IOBinaryPrimitives::writeString(testingString, bitset);
      IOBinaryPrimitives::writeBitset(bitset, file);
      file.close();
      auto val1 = std::filesystem::file_size(filename);
      auto val2 = static_cast<uintmax_t>(testingString.size()) + 1;
      CHECK(std::filesystem::file_size(filename) ==
            static_cast<uintmax_t>(testingString.size()) + 1);
    }

    DYNAMIC_SECTION("Read string (TESTING CASE: " + testingString + ")") {
      const uintmax_t startedFileSize = std::filesystem::file_size(filename);
      std::ifstream file(filename, std::ios::binary | std::ifstream::in);
      REQUIRE(file);

      std::vector<bool> unusedBits;
      std::string res = IOBinaryPrimitives::readString(file, unusedBits);
      file.close();

      CHECK(std::filesystem::file_size(filename) == startedFileSize);
      CHECK(res == testingString);
    }
  }
}

TEST_CASE("haptics::types::IOBinaryPrimitives on floats") {
  const std::vector<float> testingSet = {32, 0, -6345.365, 1.65436789};

  for (float testingFloat : testingSet) {
    DYNAMIC_SECTION("Write float (TESTING CASE: " + std::to_string(testingFloat) + ")") {
      std::ofstream file(filename, std::ios::out | std::ios::binary);
      REQUIRE(file);
      std::vector<bool> bitset;
      IOBinaryPrimitives::writeFloatNBits<uint32_t, 4 * haptics::io::BYTE_SIZE>(
          testingFloat, bitset, -haptics::io::MAX_FLOAT,
                                                        haptics::io::MAX_FLOAT);

      IOBinaryPrimitives::writeBitset(bitset, file);
      file.close();

      CHECK(std::filesystem::file_size(filename) == sizeof(float));
    }

    DYNAMIC_SECTION("Read float (TESTING CASE: " + std::to_string(testingFloat) + ")") {
      const uintmax_t startedFileSize = std::filesystem::file_size(filename);
      std::ifstream file(filename, std::ios::binary | std::ifstream::in);
      REQUIRE(file);

      std::vector<bool> unusedBits;
      float res = IOBinaryPrimitives::readFloatNBits<uint32_t, 4 * haptics::io::BYTE_SIZE>(
          file, -haptics::io::MAX_FLOAT, haptics::io::MAX_FLOAT, unusedBits);

      file.close();

      CHECK(std::filesystem::file_size(filename) == startedFileSize);
      CHECK(std::abs(res - testingFloat) < precision_threshold);
    }
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("haptics::types::IOBinaryPrimitives on 1 byte") {
  const int8_t testingValue = -42;

  SECTION("Write 1 byte") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);
    std::vector<bool> bitset;
    IOBinaryPrimitives::writeNBits<uint8_t, haptics::io::BYTE_SIZE>(testingValue, bitset);
    IOBinaryPrimitives::writeBitset(bitset, file);
    file.close();

    CHECK(std::filesystem::file_size(filename) == 1);
  }

  SECTION("Read 1 byte as signed") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary | std::ifstream::in);
    REQUIRE(file);

    std::vector<bool> unusedBits;
    auto res =
        IOBinaryPrimitives::readNBits<int8_t, haptics::io::BYTE_SIZE>(file, unusedBits);
    file.close();

    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res == testingValue);
  }

  SECTION("Read 1 byte as unsigned") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary | std::ifstream::in);
    REQUIRE(file);

    std::vector<bool> unusedBits;
    auto res =
        IOBinaryPrimitives::readNBits<uint8_t, haptics::io::BYTE_SIZE>(file, unusedBits);
    file.close();

    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res == (uint8_t)testingValue);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("haptics::types::IOBinaryPrimitives on 4 bytes") {
  const int testingValue = 1000;

  SECTION("Write 4 byte") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);
    std::vector<bool> bitset;
    IOBinaryPrimitives::writeNBits<int, 4 * haptics::io::BYTE_SIZE>(testingValue, bitset);
    IOBinaryPrimitives::writeBitset(bitset, file);
    file.close();

    CHECK(std::filesystem::file_size(filename) == 4);
  }

  SECTION("Read 4 bytes as signed") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary | std::ifstream::in);
    REQUIRE(file);

    std::vector<bool> unusedBits;
    auto res = IOBinaryPrimitives::readNBits<int32_t, 4 * haptics::io::BYTE_SIZE>(
        file, unusedBits);
    file.close();

    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res == testingValue);
  }

  SECTION("Read 4 bytes as unsigned") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary | std::ifstream::in);
    REQUIRE(file);

    std::vector<bool> unusedBits;
    auto res = IOBinaryPrimitives::readNBits<uint32_t, 4 * haptics::io::BYTE_SIZE>(
        file, unusedBits);
    file.close();

    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res == (uint32_t)testingValue);
  }

  SECTION("Read 2 bytes as signed") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary | std::ifstream::in);
    REQUIRE(file);

    std::vector<bool> unusedBits;
    auto res_part1 = IOBinaryPrimitives::readNBits<int16_t, 2 * haptics::io::BYTE_SIZE>(file, unusedBits);
    auto res_part2 = IOBinaryPrimitives::readNBits<int16_t, 2 * haptics::io::BYTE_SIZE>(file, unusedBits);
    file.close();

    const auto expectedFirstHalf = (int16_t)((testingValue & 0xFFFF0000) >> 16);
    const auto expectedSecondHalf = (int16_t)(testingValue & 0x0000FFFF);

    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res_part1 == expectedFirstHalf);
    CHECK(res_part2 == expectedSecondHalf);
  }
}
