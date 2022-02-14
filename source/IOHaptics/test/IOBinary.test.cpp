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

#include <IOHaptics/include/IOBinary.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using haptics::io::IOBinary;

const std::string filename = "testing_IOBinary.bin";

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header without avatar and perceptions") {
  const std::string testingVersion = "42";
  const std::string testingDate = "Monday, February 14, 2022";
  const std::string testingDescription =
      "I'm an awsome Haptic description placeholder and I wasn't writted by a developer";
  const std::string testingShape = "Custom";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  SECTION("write haptic header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    bool succeed = IOBinary::writeFileHeader(testingHaptic, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize = testingVersion.size() + testingDate.size() +
                                       testingDescription.size() + testingShape.size() + 4 + 2 * 2;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read band header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    bool succeed = IOBinary::readFileHeader(res, file);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getVersion() == testingVersion);
    CHECK(res.getDate() == testingDate);
    CHECK(res.getDescription() == testingDescription);
    CHECK(res.getAvatarsSize() == 0);
    CHECK(res.getPerceptionsSize() == 0);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header for avatar testing") {
  const std::string testingVersion;
  const std::string testingDate;
  const std::string testingDescription;
  const std::string testingShape = "Custom";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  const int testingId_avatar1 = 42;
  const int testingLod_avatar1 = 3;
  const auto testingType_avatar1 = haptics::types::AvatarType::Custom;
  const std::string testingMesh_avatar1 = "testing/avatar.mesh";
  haptics::types::Avatar avatar1(testingId_avatar1, testingLod_avatar1, testingType_avatar1);
  avatar1.setMesh(testingMesh_avatar1);
  testingHaptic.addAvatar(avatar1);

  const int testingId_avatar2 = 255;
  const int testingLod_avatar2 = 0;
  const auto testingType_avatar2 = haptics::types::AvatarType::Pressure;
  const std::string testingMesh_avatar2 = "placeholder";
  haptics::types::Avatar avatar2(testingId_avatar2, testingLod_avatar2, testingType_avatar2);
  avatar2.setMesh(testingMesh_avatar2);
  testingHaptic.addAvatar(avatar2);

  SECTION("write haptic header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    bool succeed = IOBinary::writeFileHeader(testingHaptic, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize = testingShape.size() + 4 + 2 * 2 + 2 * (2 + 4 + 2) + testingMesh_avatar1.size() + 1;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read band header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    bool succeed = IOBinary::readFileHeader(res, file);
    file.close();

    REQUIRE(succeed);
    REQUIRE(res.getAvatarsSize() == 2);

    CHECK(res.getAvatarAt(0).getId() == testingId_avatar1);
    CHECK(res.getAvatarAt(0).getLod() == testingLod_avatar1);
    CHECK(res.getAvatarAt(0).getType() == testingType_avatar1);
    REQUIRE(res.getAvatarAt(0).getMesh().has_value());
    CHECK(res.getAvatarAt(0).getMesh().value() == testingMesh_avatar1);

    CHECK(res.getAvatarAt(1).getId() == testingId_avatar2);
    CHECK(res.getAvatarAt(1).getLod() == testingLod_avatar2);
    CHECK(res.getAvatarAt(1).getType() == testingType_avatar2);
    CHECK_FALSE(res.getAvatarAt(1).getMesh().has_value());
  }
}