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

auto addReferenceDevice(
    haptics::types::Perception &myPerception,
    const std::vector<std::tuple<
        int, std::string, std::optional<uint32_t>, std::optional<float>, std::optional<float>,
        std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
        std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
        std::optional<float>, std::optional<haptics::types::ActuatorType>>> &referenceDeviceValues)
    -> void {
  for (auto values : referenceDeviceValues) {
    const size_t idIndex = 0;
    const size_t nameIndex = 1;
    const size_t bodyPartIndex = 2;
    const size_t maximumFrequencyIndex = 3;
    const size_t minimumFrequencyIndex = 4;
    const size_t resonanceFrequencyIndex = 5;
    const size_t maximumAmplitudeIndex = 6;
    const size_t impedanceIndex = 7;
    const size_t maximumVoltageIndex = 8;
    const size_t maximumCurrentIndex = 9;
    const size_t maximumDisplacementIndex = 10;
    const size_t weightIndex = 11;
    const size_t sizeIndex = 12;
    const size_t customIndex = 13;
    const size_t typeIndex = 14;

    haptics::types::ReferenceDevice myDevice(std::get<idIndex>(values),
                                             std::get<nameIndex>(values));

    if (std::get<bodyPartIndex>(values).has_value()) {
      myDevice.setBodyPartMask(std::get<bodyPartIndex>(values).value());
    }
    if (std::get<maximumFrequencyIndex>(values).has_value()) {
      myDevice.setMaximumFrequency(std::get<maximumFrequencyIndex>(values).value());
    }
    if (std::get<minimumFrequencyIndex>(values).has_value()) {
      myDevice.setMinimumFrequency(std::get<minimumFrequencyIndex>(values).value());
    }
    if (std::get<resonanceFrequencyIndex>(values).has_value()) {
      myDevice.setResonanceFrequency(std::get<resonanceFrequencyIndex>(values).value());
    }
    if (std::get<maximumAmplitudeIndex>(values).has_value()) {
      myDevice.setMaximumAmplitude(std::get<maximumAmplitudeIndex>(values).value());
    }
    if (std::get<impedanceIndex>(values).has_value()) {
      myDevice.setImpedance(std::get<impedanceIndex>(values).value());
    }
    if (std::get<maximumVoltageIndex>(values).has_value()) {
      myDevice.setMaximumVoltage(std::get<maximumVoltageIndex>(values).value());
    }
    if (std::get<maximumCurrentIndex>(values).has_value()) {
      myDevice.setMaximumCurrent(std::get<maximumCurrentIndex>(values).value());
    }
    if (std::get<maximumDisplacementIndex>(values).has_value()) {
      myDevice.setMaximumDisplacement(std::get<maximumDisplacementIndex>(values).value());
    }
    if (std::get<weightIndex>(values).has_value()) {
      myDevice.setWeight(std::get<weightIndex>(values).value());
    }
    if (std::get<sizeIndex>(values).has_value()) {
      myDevice.setSize(std::get<sizeIndex>(values).value());
    }
    if (std::get<customIndex>(values).has_value()) {
      myDevice.setCustom(std::get<customIndex>(values).value());
    }
    if (std::get<typeIndex>(values).has_value()) {
      myDevice.setType(std::get<typeIndex>(values).value());
    }

    myPerception.addReferenceDevice(myDevice);
  }
}

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

  SECTION("read haptic header") {
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

  SECTION("write avatars") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    bool succeed = IOBinary::writeFileHeader(testingHaptic, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize =
        testingShape.size() + 4 + 2 * 2 + 2 * (2 + 4 + 2) + testingMesh_avatar1.size() + 1;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read avatars") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    bool succeed = IOBinary::readFileHeader(res, file);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
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

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header for reference device testing") {
  const std::string testingVersion;
  const std::string testingDate;
  const std::string testingDescription;
  const std::string testingShape = "Custom";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  const int testingId_perception0 = 0;
  const int testingAvatarId_perception0 = 0;
  const std::string testingDescription_perception0 = "I'm just a random string to fill the place";
  const auto testingPerceptionModality_perception0 = haptics::types::PerceptionModality::Vibration;
  haptics::types::Perception testingPerception(testingId_perception0, testingAvatarId_perception0,
                                               testingDescription_perception0,
                                               testingPerceptionModality_perception0);

  const std::vector<std::tuple<int, std::string, std::optional<uint32_t>, std::optional<float>,
                               std::optional<float>, std::optional<float>, std::optional<float>,
                               std::optional<float>, std::optional<float>, std::optional<float>,
                               std::optional<float>, std::optional<float>, std::optional<float>,
                               std::optional<float>, std::optional<haptics::types::ActuatorType>>>
      testingReferenceDeviceValue_perception0 = {
          {-1, "This is a name", std::nullopt, 0, 1000, std::nullopt, 1, std::nullopt, std::nullopt,
           std::nullopt, std::nullopt, std::nullopt, std::nullopt, 24.42F,
           haptics::types::ActuatorType::LRA},
          {6534, "MPEG actuator", ~(uint32_t)(0), 0, 1000, 650, 1.2F, 32, 3.5F, 1000, 0.0034,
           450.0001, 543.543, 0, haptics::types::ActuatorType::Unknown},
          {0, "", std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
           std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
           std::nullopt, std::nullopt}};
  addReferenceDevice(testingPerception, testingReferenceDeviceValue_perception0);
  testingHaptic.addPerception(testingPerception);

  SECTION("write reference devices") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    bool succeed = IOBinary::writeFileHeader(testingHaptic, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize =
        testingShape.size() + testingDescription_perception0.size() +
        std::get<1>(testingReferenceDeviceValue_perception0.at(0)).size() +
        std::get<1>(testingReferenceDeviceValue_perception0.at(1)).size() +
        std::get<1>(testingReferenceDeviceValue_perception0.at(2)).size() + 110;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read reference devices") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    bool succeed = IOBinary::readFileHeader(res, file);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getPerceptionsSize() == 1);
    CHECK(res.getPerceptionAt(0).getTracksSize() == 0);
    REQUIRE(res.getPerceptionAt(0).getReferenceDevicesSize() ==
            testingReferenceDeviceValue_perception0.size());

    const size_t idIndex = 0;
    const size_t nameIndex = 1;
    const size_t bodyPartIndex = 2;
    const size_t maximumFrequencyIndex = 3;
    const size_t minimumFrequencyIndex = 4;
    const size_t resonanceFrequencyIndex = 5;
    const size_t maximumAmplitudeIndex = 6;
    const size_t impedanceIndex = 7;
    const size_t maximumVoltageIndex = 8;
    const size_t maximumCurrentIndex = 9;
    const size_t maximumDisplacementIndex = 10;
    const size_t weightIndex = 11;
    const size_t sizeIndex = 12;
    const size_t customIndex = 13;
    const size_t typeIndex = 14;
    for (int i = 0; i < static_cast<int>(res.getPerceptionAt(0).getReferenceDevicesSize()); i++) {
      haptics::types::ReferenceDevice myDevice = res.getPerceptionAt(0).getReferenceDeviceAt(i);
      std::tuple<int, std::string, std::optional<uint32_t>, std::optional<float>,
                 std::optional<float>, std::optional<float>, std::optional<float>,
                 std::optional<float>, std::optional<float>, std::optional<float>,
                 std::optional<float>, std::optional<float>, std::optional<float>,
                 std::optional<float>, std::optional<haptics::types::ActuatorType>>
          testingValues = testingReferenceDeviceValue_perception0.at(i);

      CHECK(myDevice.getId() == std::get<idIndex>(testingValues));
      CHECK(myDevice.getName() == std::get<nameIndex>(testingValues));
      CHECK(myDevice.getBodyPartMask() == (std::get<bodyPartIndex>(testingValues).has_value()
                                               ? std::get<bodyPartIndex>(testingValues).value()
                                               : 0));
      CHECK(myDevice.getMaximumFrequency() == std::get<maximumFrequencyIndex>(testingValues));
      CHECK(myDevice.getMinimumFrequency() == std::get<minimumFrequencyIndex>(testingValues));
      CHECK(myDevice.getResonanceFrequency() == std::get<resonanceFrequencyIndex>(testingValues));
      CHECK(myDevice.getMaximumAmplitude() == std::get<maximumAmplitudeIndex>(testingValues));
      CHECK(myDevice.getImpedance() == std::get<impedanceIndex>(testingValues));
      CHECK(myDevice.getMaximumVoltage() == std::get<maximumVoltageIndex>(testingValues));
      CHECK(myDevice.getMaximumCurrent() == std::get<maximumCurrentIndex>(testingValues));
      CHECK(myDevice.getMaximumDisplacement() == std::get<maximumDisplacementIndex>(testingValues));
      CHECK(myDevice.getWeight() == std::get<weightIndex>(testingValues));
      CHECK(myDevice.getSize() == std::get<sizeIndex>(testingValues));
      CHECK(myDevice.getType() == std::get<typeIndex>(testingValues));
    }
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header for track testing") {
  const std::string testingVersion;
  const std::string testingDate;
  const std::string testingDescription;
  const std::string testingShape = "Custom";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  const int testingId_perception0 = 0;
  const int testingAvatarId_perception0 = 0;
  const std::string testingDescription_perception0 = "I'm just a random string to fill the place";
  const auto testingPerceptionModality_perception0 = haptics::types::PerceptionModality::Vibration;
  haptics::types::Perception testingPerception0(testingId_perception0, testingAvatarId_perception0,
                                                testingDescription_perception0,
                                                testingPerceptionModality_perception0);

  testingHaptic.addPerception(testingPerception0);

  SECTION("write tracks header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    bool succeed = IOBinary::writeFileHeader(testingHaptic, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize = testingShape.size() + testingDescription_perception0.size();
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read track header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    bool succeed = IOBinary::readFileHeader(res, file);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getPerceptionsSize() == 1);
    CHECK(res.getPerceptionAt(0).getTracksSize() == 0);
  }
}
