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
#include <IOHaptics/include/IOBinaryFields.h>
#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using haptics::io::IOBinary;
using haptics::io::IOBinaryPrimitives;

const std::string filename = "testing_IOBinary.bin";
constexpr float floatPrecision = 0.01;

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header without avatar and perceptions") {
  const std::string testingVersion = "42";
  const std::string testingDate = "Monday, February 14, 2022";
  std::string testingProfile = "Simple Parametric";
  const uint8_t testingLevel = 2;
  const std::string testingDescription =
      "I'm an awsome Haptic description placeholder and I wasn't writted by a developer";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);
  testingHaptic.setProfile(testingProfile);
  testingHaptic.setLevel(testingLevel);

  SECTION("write haptic header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    bool succeed = IOBinary::writeFileHeader(testingHaptic, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    REQUIRE(succeed);
    auto bitStreamSize = (testingVersion.size() + testingProfile.size() + testingDate.size() +
                          testingDescription.size() + 4) *
                             haptics::io::BYTE_SIZE +
                         haptics::io::LEVEL + haptics::io::MDEXP_AVATAR_COUNT +
                         haptics::io::MDEXP_PERC_COUNT;
    auto byteStreamSize = bitStreamSize % haptics::io::BYTE_SIZE == 0
                              ? bitStreamSize / haptics::io::BYTE_SIZE
                              : (bitStreamSize / haptics::io::BYTE_SIZE) + 1;
    CHECK(std::filesystem::file_size(filename) == static_cast<uintmax_t>(byteStreamSize));
  }

  SECTION("read haptic header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;

    std::vector<bool> unusedBits;
    bool succeed = IOBinary::readFileHeader(res, file, unusedBits);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getVersion() == testingVersion);
    CHECK(res.getDate() == testingDate);
    CHECK(res.getProfile() == testingProfile);
    CHECK(res.getLevel() == testingLevel);
    CHECK(res.getDescription() == testingDescription);
    CHECK(res.getAvatarsSize() == 0);
    CHECK(res.getPerceptionsSize() == 0);

    std::filesystem::remove(filename);
    CHECK(!std::filesystem::is_regular_file(filename));
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header for avatar testing") {
  const std::string testingVersion;
  const std::string testingDate;
  std::string testingProfile = "Main";
  const uint8_t testingLevel = 1;
  const std::string testingDescription;
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);
  testingHaptic.setProfile(testingProfile);
  testingHaptic.setLevel(testingLevel);

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

    std::vector<bool> output;
    bool succeed = IOBinary::writeFileHeader(testingHaptic, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    REQUIRE(succeed);

    auto bitStreamSize =
        static_cast<int>(testingVersion.size() + testingProfile.size() + testingDate.size() +
                         testingDescription.size() + 4) *
            haptics::io::BYTE_SIZE +
        haptics::io::LEVEL +
        2 * (haptics::io::AVATAR_ID + haptics::io::AVATAR_LOD + haptics::io::AVATAR_TYPE) +
        haptics::io::MDEXP_AVATAR_COUNT + haptics::io::MDEXP_PERC_COUNT +
        (testingMesh_avatar1.size() + 1) * haptics::io::BYTE_SIZE;
    auto byteStreamSize = bitStreamSize % haptics::io::BYTE_SIZE == 0
                              ? bitStreamSize / haptics::io::BYTE_SIZE
                              : (bitStreamSize / haptics::io::BYTE_SIZE) + 1;

    CHECK(std::filesystem::file_size(filename) == static_cast<uintmax_t>(byteStreamSize));
  }

  SECTION("read avatars") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinary::readFileHeader(res, file, unusedBits);
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

    std::filesystem::remove(filename);
    CHECK(!std::filesystem::is_regular_file(filename));
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header for reference device testing") {
  const std::string testingVersion;
  const std::string testingDate;
  const std::string testingDescription;
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  const int testingId_perception0 = 0;
  const int testingAvatarId_perception0 = 0;
  const std::string testingDescription_perception0 = "I'm just a random string to fill the place";
  const auto testingPerceptionModality_perception0 =
      haptics::types::PerceptionModality::Vibrotactile;
  haptics::types::Perception testingPerception(testingId_perception0, testingAvatarId_perception0,
                                               testingDescription_perception0,
                                               testingPerceptionModality_perception0);

  const std::vector<std::tuple<int, std::string, std::optional<uint32_t>, std::optional<float>,
                               std::optional<float>, std::optional<float>, std::optional<float>,
                               std::optional<float>, std::optional<float>, std::optional<float>,
                               std::optional<float>, std::optional<float>, std::optional<float>,
                               std::optional<float>, std::optional<haptics::types::ActuatorType>>>
      testingReferenceDeviceValue_perception0 = {
          {0, "This is a name", std::nullopt, 0, 1000, std::nullopt, 1, std::nullopt, std::nullopt,
           std::nullopt, std::nullopt, std::nullopt, std::nullopt, 24.42F,
           haptics::types::ActuatorType::LRA},
          {134, "MPEG actuator", ~(uint32_t)(0), 0, 1000, 650, 1.2F, 32, 3.5F, 1000, 0.0034,
           450.0001, 543.543, 0, haptics::types::ActuatorType::Unknown},
          {0, "", std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
           std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
           std::nullopt, std::nullopt}};
  testingPerception.addReferenceDevice(testingReferenceDeviceValue_perception0);
  testingHaptic.addPerception(testingPerception);

  SECTION("write reference devices") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);
    std::vector<bool> output;
    bool succeed = IOBinary::writeFileHeader(testingHaptic, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize =
        testingDescription_perception0.size() +
        std::get<1>(testingReferenceDeviceValue_perception0.at(0)).size() +
        std::get<1>(testingReferenceDeviceValue_perception0.at(1)).size() +
        std::get<1>(testingReferenceDeviceValue_perception0.at(2)).size() + 106;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read reference devices") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinary::readFileHeader(res, file, unusedBits);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getPerceptionsSize() == 1);
    CHECK(res.getPerceptionAt(0).getChannelsSize() == 0);
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

      CHECK(myDevice.getMaximumFrequency().has_value() ==
            std::get<maximumFrequencyIndex>(testingValues).has_value());
      if (myDevice.getMaximumFrequency().has_value()) {
        CHECK(std::fabs(myDevice.getMaximumFrequency().value() -
                        std::get<maximumFrequencyIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getMinimumFrequency().has_value() ==
            std::get<minimumFrequencyIndex>(testingValues).has_value());
      if (myDevice.getMinimumFrequency().has_value()) {
        CHECK(std::fabs(myDevice.getMinimumFrequency().value() -
                        std::get<minimumFrequencyIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getResonanceFrequency().has_value() ==
            std::get<resonanceFrequencyIndex>(testingValues).has_value());
      if (myDevice.getResonanceFrequency().has_value()) {
        CHECK(std::fabs(myDevice.getResonanceFrequency().value() -
                        std::get<resonanceFrequencyIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getMaximumAmplitude().has_value() ==
            std::get<maximumAmplitudeIndex>(testingValues).has_value());
      if (myDevice.getMaximumAmplitude().has_value()) {
        CHECK(std::fabs(myDevice.getMaximumAmplitude().value() -
                        std::get<maximumAmplitudeIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getImpedance().has_value() ==
            std::get<impedanceIndex>(testingValues).has_value());
      if (myDevice.getImpedance().has_value()) {
        CHECK(std::fabs(myDevice.getImpedance().value() -
                        std::get<impedanceIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getMaximumVoltage().has_value() ==
            std::get<maximumVoltageIndex>(testingValues).has_value());
      if (myDevice.getMaximumVoltage().has_value()) {
        CHECK(std::fabs(myDevice.getMaximumVoltage().value() -
                        std::get<maximumVoltageIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getMaximumCurrent().has_value() ==
            std::get<maximumCurrentIndex>(testingValues).has_value());
      if (myDevice.getMaximumCurrent().has_value()) {
        CHECK(std::fabs(myDevice.getMaximumCurrent().value() -
                        std::get<maximumCurrentIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getMaximumDisplacement().has_value() ==
            std::get<maximumDisplacementIndex>(testingValues).has_value());
      if (myDevice.getMaximumDisplacement().has_value()) {
        CHECK(std::fabs(myDevice.getMaximumDisplacement().value() -
                        std::get<maximumDisplacementIndex>(testingValues).value()) <
              floatPrecision);
      }

      CHECK(myDevice.getWeight().has_value() == std::get<weightIndex>(testingValues).has_value());
      if (myDevice.getWeight().has_value()) {
        CHECK(std::fabs(myDevice.getWeight().value() -
                        std::get<weightIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getSize().has_value() == std::get<sizeIndex>(testingValues).has_value());
      if (myDevice.getSize().has_value()) {
        CHECK(std::fabs(myDevice.getSize().value() - std::get<sizeIndex>(testingValues).value()) <
              floatPrecision);
      }

      CHECK(myDevice.getCustom().has_value() == std::get<customIndex>(testingValues).has_value());
      if (myDevice.getCustom().has_value()) {
        CHECK(std::fabs(myDevice.getCustom().value() -
                        std::get<customIndex>(testingValues).value()) < floatPrecision);
      }

      CHECK(myDevice.getType() == std::get<typeIndex>(testingValues));
    }

    std::filesystem::remove(filename);
    CHECK(!std::filesystem::is_regular_file(filename));
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file header for channel testing") {
  const std::string testingVersion;
  const std::string testingDate;
  const std::string testingDescription;
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  const int testingId_perception0 = 0;
  const int testingAvatarId_perception0 = 0;
  const std::string testingDescription_perception0 = "I'm just a random string to fill the place";
  const auto testingPerceptionModality_perception0 =
      haptics::types::PerceptionModality::Vibrotactile;
  haptics::types::Perception testingPerception0(testingId_perception0, testingAvatarId_perception0,
                                                testingDescription_perception0,
                                                testingPerceptionModality_perception0);

  const int testingId_perception1 = 123;
  const int testingAvatarId_perception1 = 3;
  const std::string testingDescription_perception1 = "This developer need an HAPTIC coffee !";
  const auto testingPerceptionModality_perception1 = haptics::types::PerceptionModality::Other;
  haptics::types::Perception testingPerception1(testingId_perception1, testingAvatarId_perception1,
                                                testingDescription_perception1,
                                                testingPerceptionModality_perception1);

  const int testingId_channel0 = 0;
  const std::string testingDescription_channel0 = "testingDescription_channel0";
  const float testingGain_channel0 = .34;
  const float testingMixingWeight_channel0 = 1;
  const uint32_t testingBodyPartMask_channel0 = 32;
  const std::vector<int> testingVertices_channel0 = {0, 453, -3, 7657};
  const size_t testingBandsCount_channel0 = 45;
  const haptics::types::Vector testingDirection_channel0((int8_t)0, (int8_t)128, (int8_t)-42);
  haptics::types::Channel testingChannel0(testingId_channel0, testingDescription_channel0,
                                          testingGain_channel0, testingMixingWeight_channel0,
                                          testingBodyPartMask_channel0);
  testingChannel0.setDirection(testingDirection_channel0);
  for (auto vertex : testingVertices_channel0) {
    testingChannel0.addVertex(vertex);
  }
  for (size_t i = 0; i < testingBandsCount_channel0; i++) {
    testingChannel0.generateBand();
  }

  const int testingId_channel1 = 132;
  const std::string testingDescription_channel1 = "again another string";
  const float testingGain_channel1 = 0;
  const float testingMixingWeight_channel1 = .333;
  const uint32_t testingBodyPartMask_channel1 = ~(uint32_t)(0);
  const size_t testingBandsCount_channel1 = 0;
  haptics::types::Channel testingChannel1(testingId_channel1, testingDescription_channel1,
                                          testingGain_channel1, testingMixingWeight_channel1,
                                          testingBodyPartMask_channel1);
  for (size_t i = 0; i < testingBandsCount_channel1; i++) {
    testingChannel1.generateBand();
  }

  const int testingId_channel2 = 4;
  const std::string testingDescription_channel2 = "I'm inside a test";
  const float testingGain_channel2 = 2.7652;
  const float testingMixingWeight_channel2 = .6666;
  const uint32_t testingBodyPartMask_channel2 = 0;
  const std::vector<int> testingVertices_channel2 = {0, 6};
  const size_t testingBandsCount_channel2 = 1;
  const haptics::types::Vector testingChannelResolution_channel2(32, 110, 3);
  const std::vector<haptics::types::Vector> testingActuatorTarget_channel2{
      haptics::types::Vector{31, 109, 2},
      haptics::types::Vector{0, 0, 0},
      haptics::types::Vector{15, 42, 1},
  };
  const std::vector<haptics::types::BodyPartTarget> testingBodyPartTarget_channel2{
      haptics::types::BodyPartTarget::Left,         haptics::types::BodyPartTarget::Index,
      haptics::types::BodyPartTarget::ThirdPhalanx, haptics::types::BodyPartTarget::Plus,
      haptics::types::BodyPartTarget::Right,        haptics::types::BodyPartTarget::Leg,
      haptics::types::BodyPartTarget::Minus,        haptics::types::BodyPartTarget::Hallux,
  };
  haptics::types::Channel testingChannel2(testingId_channel2, testingDescription_channel2,
                                          testingGain_channel2, testingMixingWeight_channel2,
                                          testingBodyPartMask_channel2);
  for (auto vertex : testingVertices_channel2) {
    testingChannel2.addVertex(vertex);
  }
  for (size_t i = 0; i < testingBandsCount_channel2; i++) {
    testingChannel2.generateBand();
  }
  testingChannel2.setActuatorResolution(testingChannelResolution_channel2);
  testingChannel2.setActuatorTarget(testingActuatorTarget_channel2);
  testingChannel2.setBodyPartTarget(testingBodyPartTarget_channel2);

  testingPerception0.addChannel(testingChannel0);
  testingPerception0.addChannel(testingChannel1);
  testingPerception1.addChannel(testingChannel2);
  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);

  SECTION("write channels header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);
    std::vector<bool> output;
    bool succeed = IOBinary::writeFileHeader(testingHaptic, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    REQUIRE(succeed);
    const uintmax_t expectedFileSize =
        testingDescription_perception0.size() + testingDescription_perception1.size() +
        testingDescription_channel0.size() + testingDescription_channel1.size() +
        testingDescription_channel2.size() + testingVertices_channel0.size() +
        testingVertices_channel2.size() + 144;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read channel header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Haptics res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinary::readFileHeader(res, file, unusedBits);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getPerceptionsSize() == 2);
    REQUIRE(res.getPerceptionAt(0).getChannelsSize() == 2);
    CHECK(res.getPerceptionAt(0).getId() == testingId_perception0);
    CHECK(res.getPerceptionAt(0).getAvatarId() == testingAvatarId_perception0);
    CHECK(res.getPerceptionAt(0).getDescription() == testingDescription_perception0);
    CHECK(res.getPerceptionAt(0).getPerceptionModality() == testingPerceptionModality_perception0);

    CHECK(res.getPerceptionAt(0).getChannelAt(0).getId() == testingId_channel0);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getDescription() == testingDescription_channel0);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(0).getGain() - testingGain_channel0) <
          floatPrecision);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(0).getMixingWeight() -
                    testingMixingWeight_channel0));
    REQUIRE(res.getPerceptionAt(0).getChannelAt(0).getVerticesSize() ==
            testingVertices_channel0.size());
    for (int i = 0; i < static_cast<int>(testingVertices_channel0.size()); i++) {
      REQUIRE(res.getPerceptionAt(0).getChannelAt(0).getVertexAt(i) ==
              testingVertices_channel0.at(i));
    }
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandsSize() == testingBandsCount_channel0);
    REQUIRE(res.getPerceptionAt(0).getChannelAt(0).getDirection().has_value());
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getDirection().value() ==
          testingDirection_channel0);
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(0).getActuatorResolution().has_value());
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(0).getBodyPartTarget().has_value());
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(0).getActuatorTarget().has_value());

    CHECK(res.getPerceptionAt(0).getChannelAt(1).getId() == testingId_channel1);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getDescription() == testingDescription_channel1);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(1).getGain() - testingGain_channel1) <
          floatPrecision);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(1).getMixingWeight() -
                    testingMixingWeight_channel1) < floatPrecision);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getVerticesSize() == 0);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getBandsSize() == testingBandsCount_channel1);
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(1).getDirection().has_value());
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(1).getActuatorResolution().has_value());
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(1).getBodyPartTarget().has_value());
    CHECK_FALSE(res.getPerceptionAt(0).getChannelAt(1).getActuatorTarget().has_value());

    REQUIRE(res.getPerceptionAt(1).getChannelsSize() == 1);
    CHECK(res.getPerceptionAt(1).getId() == testingId_perception1);
    CHECK(res.getPerceptionAt(1).getAvatarId() == testingAvatarId_perception1);
    CHECK(res.getPerceptionAt(1).getDescription() == testingDescription_perception1);
    CHECK(res.getPerceptionAt(1).getPerceptionModality() == testingPerceptionModality_perception1);

    CHECK(res.getPerceptionAt(1).getChannelAt(0).getId() == testingId_channel2);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getDescription() == testingDescription_channel2);
    CHECK(std::fabs(res.getPerceptionAt(1).getChannelAt(0).getGain() - testingGain_channel2) <
          floatPrecision);
    CHECK(std::fabs(res.getPerceptionAt(1).getChannelAt(0).getMixingWeight() -
                    testingMixingWeight_channel2) < floatPrecision);
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getVerticesSize() ==
            testingVertices_channel2.size());
    for (int i = 0; i < static_cast<int>(testingVertices_channel2.size()); i++) {
      REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getVertexAt(i) ==
              testingVertices_channel2.at(i));
    }
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBandsSize() == testingBandsCount_channel2);
    CHECK_FALSE(res.getPerceptionAt(1).getChannelAt(0).getDirection().has_value());
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBodyPartMask() == 0);
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getActuatorResolution().has_value());
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getActuatorResolution().value() ==
          testingChannelResolution_channel2);
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getBodyPartTarget().has_value());
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getBodyPartTarget().value().size() ==
            testingBodyPartTarget_channel2.size());
    for (size_t i = 0; i < testingBodyPartTarget_channel2.size(); i++) {
      CHECK(res.getPerceptionAt(1).getChannelAt(0).getBodyPartTarget().value()[i] ==
            testingBodyPartTarget_channel2[i]);
    }
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getActuatorTarget().has_value());
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getActuatorTarget().value().size() ==
            testingActuatorTarget_channel2.size());
    for (size_t i = 0; i < testingActuatorTarget_channel2.size(); i++) {
      CHECK(res.getPerceptionAt(1).getChannelAt(0).getActuatorTarget().value()[i] ==
            testingActuatorTarget_channel2[i]);
    }

    std::filesystem::remove(filename);
    CHECK(!std::filesystem::is_regular_file(filename));
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read file for body testing") {
  const std::string testingVersion = "2023";
  const std::string testingDate = "2023-12-15";
  const std::string testingDescription = "I'm a testing value";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

  const int testingId_avatar1 = 42;
  const int testingLod_avatar1 = 3;
  const auto testingType_avatar1 = haptics::types::AvatarType::Custom;
  const std::string testingMesh_avatar1 = "testing/avatar.mesh";
  haptics::types::Avatar avatar1(testingId_avatar1, testingLod_avatar1, testingType_avatar1);
  avatar1.setMesh(testingMesh_avatar1);

  const int testingId_avatar2 = 255;
  const int testingLod_avatar2 = 0;
  const auto testingType_avatar2 = haptics::types::AvatarType::Pressure;
  const std::string testingMesh_avatar2 = "placeholder";
  haptics::types::Avatar avatar2(testingId_avatar2, testingLod_avatar2, testingType_avatar2);
  avatar2.setMesh(testingMesh_avatar2);

  const int testingId_perception0 = 0;
  const int testingAvatarId_perception0 = 0;
  const std::string testingDescription_perception0 = "I'm just a random string to fill the place";
  const auto testingPerceptionModality_perception0 =
      haptics::types::PerceptionModality::Vibrotactile;
  haptics::types::Perception testingPerception0(testingId_perception0, testingAvatarId_perception0,
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
  testingPerception0.addReferenceDevice(testingReferenceDeviceValue_perception0);

  const int testingId_perception1 = 123;
  const int testingAvatarId_perception1 = 3;
  const std::string testingDescription_perception1 = "This developer need an HAPTIC coffee !";
  const auto testingPerceptionModality_perception1 = haptics::types::PerceptionModality::Other;
  haptics::types::Perception testingPerception1(testingId_perception1, testingAvatarId_perception1,
                                                testingDescription_perception1,
                                                testingPerceptionModality_perception1);

  const int testingId_channel0 = 0;
  const std::string testingDescription_channel0 = "testingDescription_channel0";
  const float testingGain_channel0 = .34;
  const float testingMixingWeight_channel0 = 1;
  const uint32_t testingBodyPartMask_channel0 = 32;
  const std::vector<int> testingVertices_channel0 = {0, 453, -3, 7657};
  haptics::types::Channel testingChannel0(testingId_channel0, testingDescription_channel0,
                                          testingGain_channel0, testingMixingWeight_channel0,
                                          testingBodyPartMask_channel0);
  for (auto vertex : testingVertices_channel0) {
    testingChannel0.addVertex(vertex);
  }

  const int testingId_channel1 = 132;
  const std::string testingDescription_channel1 = "again another string";
  const float testingGain_channel1 = 0;
  const float testingMixingWeight_channel1 = .333;
  const uint32_t testingBodyPartMask_channel1 = ~(uint32_t)(0);
  haptics::types::Channel testingChannel1(testingId_channel1, testingDescription_channel1,
                                          testingGain_channel1, testingMixingWeight_channel1,
                                          testingBodyPartMask_channel1);

  const int testingId_channel2 = 4;
  const std::string testingDescription_channel2 = "I'm inside a test";
  const float testingGain_channel2 = 2.7652;
  const float testingMixingWeight_channel2 = .6666;
  const uint32_t testingBodyPartMask_channel2 = 0;
  const std::vector<int> testingVertices_channel2 = {0, 6};
  haptics::types::Channel testingChannel2(testingId_channel2, testingDescription_channel2,
                                          testingGain_channel2, testingMixingWeight_channel2,
                                          testingBodyPartMask_channel2);
  for (auto vertex : testingVertices_channel2) {
    testingChannel2.addVertex(vertex);
  }

  const auto testingBandType_band0 = haptics::types::BandType::Curve;
  const auto testingCurveType_band0 = haptics::types::CurveType::Cubic;
  const int testingLowerFrequencyLimit_band0 = 0;
  const int testingUpperFrequencyLimit_band0 = 75;
  haptics::types::Band testingBand0(testingBandType_band0, testingCurveType_band0,
                                    testingLowerFrequencyLimit_band0,
                                    testingUpperFrequencyLimit_band0);

  const auto testingBandType_band1 = haptics::types::BandType::Transient;
  const int testingLowerFrequencyLimit_band1 = 65;
  const int testingUpperFrequencyLimit_band1 = 300;
  haptics::types::Band testingBand1(testingBandType_band1, testingLowerFrequencyLimit_band1,
                                    testingUpperFrequencyLimit_band1);

  const auto testingBandType_band2 = haptics::types::BandType::VectorialWave;
  const int testingLowerFrequencyLimit_band2 = 0;
  const int testingUpperFrequencyLimit_band2 = 1000;
  haptics::types::Band testingBand2(testingBandType_band2, testingLowerFrequencyLimit_band2,
                                    testingUpperFrequencyLimit_band2);

  const int testingPosition_effect0 = 63;
  const float testingPhase_effect0 = 0;
  const auto testingBaseSignal_effect0 = haptics::types::BaseSignal::Sine;
  const std::vector<std::tuple<int, float>> testingKeyframes_effect0 = {
      {0, 0}, {176, .2143543}, {177, 1}, {52345, .453}};
  haptics::types::Effect testingEffect0(testingPosition_effect0, testingPhase_effect0,
                                        testingBaseSignal_effect0,
                                        haptics::types::EffectType::Basis);
  for (auto value : testingKeyframes_effect0) {
    testingEffect0.addAmplitudeAt(std::get<1>(value), std::get<0>(value));
  }

  const std::vector<std::tuple<int, float, int>> testingKeyframes_effect1 = {
      {0, 0, 90}, {176, .2143543, 90}, {177, 1, 65}, {52345, .453, 300}};
  for (auto value : testingKeyframes_effect1) {
    haptics::types::Effect testingEffect(std::get<0>(value), 0, haptics::types::BaseSignal::Sine,
                                         haptics::types::EffectType::Basis);
    testingEffect.addAmplitudeAt(std::get<1>(value), 0);
    testingEffect.addFrequencyAt(std::get<2>(value), 0);
    testingBand1.addEffect(testingEffect);
  }

  const int testingPosition_effect2 = 6522;
  const float testingPhase_effect2 = 0;
  const auto testingBaseSignal_effect2 = haptics::types::BaseSignal::Square;
  haptics::types::Effect testingEffect2(testingPosition_effect2, testingPhase_effect2,
                                        testingBaseSignal_effect2,
                                        haptics::types::EffectType::Basis);

  testingBand0.addEffect(testingEffect0);
  testingBand2.addEffect(testingEffect2);
  testingChannel0.addBand(testingBand0);
  testingChannel1.addBand(testingBand1);
  testingChannel2.addBand(testingBand2);
  testingPerception0.addChannel(testingChannel0);
  testingPerception0.addChannel(testingChannel1);
  testingPerception1.addChannel(testingChannel2);
  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);
  testingHaptic.addAvatar(avatar1);
  testingHaptic.addAvatar(avatar2);

  SECTION("write file") {
    bool succeed = IOBinary::writeFile(testingHaptic, filename);

    REQUIRE(succeed);
    const uintmax_t expectedFileSize =
        testingVersion.size() + testingDate.size() + testingDescription.size() +
        testingMesh_avatar1.size() + testingMesh_avatar2.size() +
        testingDescription_perception0.size() + testingDescription_perception1.size() +
        testingDescription_channel0.size() + testingDescription_channel1.size() +
        testingDescription_channel2.size() + testingVertices_channel0.size() +
        testingVertices_channel2.size() + 316;
    CHECK(std::filesystem::file_size(filename) == expectedFileSize);
  }

  SECTION("read file") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);

    haptics::types::Haptics res;
    bool succeed = IOBinary::loadFile(filename, res);

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getVersion() == testingVersion);
    CHECK(res.getDate() == testingDate);
    CHECK(res.getDescription() == testingDescription);

    // CHECK avatars
    REQUIRE(res.getAvatarsSize() == 2);
    CHECK(res.getAvatarAt(0).getId() == testingId_avatar1);
    CHECK(res.getAvatarAt(0).getLod() == testingLod_avatar1);
    CHECK(res.getAvatarAt(0).getType() == testingType_avatar1);
    CHECK(res.getAvatarAt(0).getMesh() == testingMesh_avatar1);
    CHECK(res.getAvatarAt(1).getId() == testingId_avatar2);
    CHECK(res.getAvatarAt(1).getLod() == testingLod_avatar2);
    CHECK(res.getAvatarAt(1).getType() == testingType_avatar2);

    REQUIRE(res.getPerceptionsSize() == 2);

    // CHECK perception 0
    CHECK(res.getPerceptionAt(0).getAvatarId() == testingAvatarId_perception0);
    CHECK(res.getPerceptionAt(0).getDescription() == testingDescription_perception0);
    CHECK(res.getPerceptionAt(0).getId() == testingId_perception0);
    CHECK(res.getPerceptionAt(0).getPerceptionModality() == testingPerceptionModality_perception0);
    REQUIRE(res.getPerceptionAt(0).getChannelsSize() == 2);

    // CHECK channel 0
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBodyPartMask() == testingBodyPartMask_channel0);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getDescription() == testingDescription_channel0);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(0).getGain() - testingGain_channel0) <
          floatPrecision);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getId() == testingId_channel0);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(0).getMixingWeight() -
                    testingMixingWeight_channel0) < floatPrecision);
    REQUIRE(res.getPerceptionAt(0).getChannelAt(0).getVerticesSize() ==
            testingVertices_channel0.size());
    for (int i = 0; i < static_cast<int>(testingVertices_channel0.size()); i++) {
      CHECK(res.getPerceptionAt(0).getChannelAt(0).getVertexAt(i) ==
            testingVertices_channel0.at(i));
    }

    // CHECK band
    REQUIRE(res.getPerceptionAt(0).getChannelAt(0).getBandsSize() == 1);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getBandType() ==
          testingBandType_band0);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getCurveType() ==
          testingCurveType_band0);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getLowerFrequencyLimit() ==
          testingLowerFrequencyLimit_band0);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getUpperFrequencyLimit() ==
          testingUpperFrequencyLimit_band0);
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getEffectsSize() == 1);

    // CHECK effect
    CHECK(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getEffectAt(0).getPosition() ==
          testingPosition_effect0);
    CHECK(
        std::fabs(
            res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getEffectAt(0).getPhaseOrDefault() -
            testingPhase_effect0) < floatPrecision);
    CHECK(res.getPerceptionAt(0)
              .getChannelAt(0)
              .getBandAt(0)
              .getEffectAt(0)
              .getBaseSignalOrDefault() == testingBaseSignal_effect0);
    REQUIRE(res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getEffectAt(0).getKeyframesSize() ==
            testingKeyframes_effect0.size());
    for (int i = 0; i < static_cast<int>(testingKeyframes_effect0.size()); i++) {
      haptics::types::Keyframe resKeyframe =
          res.getPerceptionAt(0).getChannelAt(0).getBandAt(0).getEffectAt(0).getKeyframeAt(i);
      REQUIRE(resKeyframe.getRelativePosition().has_value());
      REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
      CHECK_FALSE(resKeyframe.getFrequencyModulation().has_value());
      CHECK(resKeyframe.getRelativePosition().value() ==
            std::get<0>(testingKeyframes_effect0.at(i)));
      CHECK(std::fabs(resKeyframe.getAmplitudeModulation().value() -
                      std::get<1>(testingKeyframes_effect0.at(i))) < floatPrecision);
    }

    // CHECK channel 1
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getBodyPartMask() == testingBodyPartMask_channel1);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getDescription() == testingDescription_channel1);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(1).getGain() - testingGain_channel1) <
          floatPrecision);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getId() == testingId_channel1);
    CHECK(std::fabs(res.getPerceptionAt(0).getChannelAt(1).getMixingWeight() -
                    testingMixingWeight_channel1) < floatPrecision);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getVerticesSize() == 0);

    // CHECK band
    REQUIRE(res.getPerceptionAt(0).getChannelAt(1).getBandsSize() == 1);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getBandAt(0).getBandType() ==
          testingBandType_band1);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getBandAt(0).getLowerFrequencyLimit() ==
          testingLowerFrequencyLimit_band1);
    CHECK(res.getPerceptionAt(0).getChannelAt(1).getBandAt(0).getUpperFrequencyLimit() ==
          testingUpperFrequencyLimit_band1);
    REQUIRE(res.getPerceptionAt(0).getChannelAt(1).getBandAt(0).getEffectsSize() ==
            testingKeyframes_effect1.size());

    // CHECK effects
    for (int i = 0; i < static_cast<int>(testingKeyframes_effect1.size()); i++) {
      haptics::types::Effect resEffect =
          res.getPerceptionAt(0).getChannelAt(1).getBandAt(0).getEffectAt(i);
      CHECK(resEffect.getPosition() == std::get<0>(testingKeyframes_effect1.at(i)));
      CHECK(std::fabs(resEffect.getPhaseOrDefault() - 0) < floatPrecision);
      REQUIRE(resEffect.getKeyframesSize() == 1);
      REQUIRE(resEffect.getKeyframeAt(0).getRelativePosition().has_value());
      REQUIRE(resEffect.getKeyframeAt(0).getAmplitudeModulation().has_value());
      REQUIRE(resEffect.getKeyframeAt(0).getFrequencyModulation().has_value());
      CHECK(resEffect.getKeyframeAt(0).getRelativePosition().value() == 0);
      CHECK(std::fabs(resEffect.getKeyframeAt(0).getAmplitudeModulation().value() -
                      std::get<1>(testingKeyframes_effect1.at(i))) < floatPrecision);
      CHECK(resEffect.getKeyframeAt(0).getFrequencyModulation().value() ==
            std::get<2>(testingKeyframes_effect1.at(i)));
    }

    // CHECK perception 1
    CHECK(res.getPerceptionAt(1).getAvatarId() == testingAvatarId_perception1);
    CHECK(res.getPerceptionAt(1).getDescription() == testingDescription_perception1);
    CHECK(res.getPerceptionAt(1).getId() == testingId_perception1);
    CHECK(res.getPerceptionAt(1).getPerceptionModality() == testingPerceptionModality_perception1);
    CHECK(res.getPerceptionAt(1).getReferenceDevicesSize() == 0);
    REQUIRE(res.getPerceptionAt(1).getChannelsSize() == 1);

    // CHECK channel
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBodyPartMask() == testingBodyPartMask_channel2);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getDescription() == testingDescription_channel2);
    CHECK(std::fabs(res.getPerceptionAt(1).getChannelAt(0).getGain() - testingGain_channel2) <
          floatPrecision);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getId() == testingId_channel2);
    CHECK(std::fabs(res.getPerceptionAt(1).getChannelAt(0).getMixingWeight() -
                    testingMixingWeight_channel2) < floatPrecision);
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getVerticesSize() ==
            testingVertices_channel2.size());
    for (int i = 0; i < static_cast<int>(testingVertices_channel2.size()); i++) {
      CHECK(res.getPerceptionAt(1).getChannelAt(0).getVertexAt(i) ==
            testingVertices_channel2.at(i));
    }

    // CHECK band
    REQUIRE(res.getPerceptionAt(1).getChannelAt(0).getBandsSize() == 1);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBandAt(0).getBandType() ==
          testingBandType_band2);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBandAt(0).getLowerFrequencyLimit() ==
          testingLowerFrequencyLimit_band2);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBandAt(0).getUpperFrequencyLimit() ==
          testingUpperFrequencyLimit_band2);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBandAt(0).getEffectsSize() == 1);
    CHECK(res.getPerceptionAt(1).getChannelAt(0).getBandAt(0).getEffectAt(0).getKeyframesSize() ==
          0);

    std::filesystem::remove(filename);
    CHECK(!std::filesystem::is_regular_file(filename));
  }
}