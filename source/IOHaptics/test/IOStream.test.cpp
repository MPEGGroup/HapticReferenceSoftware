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

#include <IOHaptics/include/IOStream.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using haptics::io::IOStream;

const std::string filename = "testing_IOStream.bin";
constexpr float floatPrecision = 0.01;
constexpr size_t bl = 512;
constexpr int BITS_EFFECT = 15;
constexpr int MOD_VAL = 256;
constexpr int PACKET_NUMBER = 8;
constexpr float scalar = 1.5;
constexpr int PACKET_DURATION = 128;

//  NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
//  NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("Write/Read Haptic databand as streamable packet") {
  using haptics::spiht::ArithEnc;
  using haptics::spiht::Spiht_Dec;
  using haptics::spiht::Spiht_Enc;
  using haptics::types::Effect;

  const std::string testingVersion = "RM1";
  const std::string testingDate = "Today";
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

  const int testingId_perception1 = 255;
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

  const int testingId_channel1 = 255;
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

  const auto testingBandType_band2 = haptics::types::BandType::WaveletWave;
  const int testingWindowLength_band2 = 128;
  const int testingLowerFrequencyLimit_band2 = 0;
  const int testingUpperFrequencyLimit_band2 = 1000;
  haptics::types::Band testingBand2(testingBandType_band2, testingWindowLength_band2,
                                    testingLowerFrequencyLimit_band2,
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
      {0, 0, 90}, {176, .2143543, 90}, {1024, 1, 65}, {52345, .453, 300}};
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
  Effect effect_in;
  for (size_t i = 0; i < bl; i++) {
    Keyframe keyframe(i, (float)(i % MOD_VAL) / MOD_VAL, 0);
    effect_in.addKeyframe(keyframe);
  }
  Keyframe keyframe(bl, scalar, 0);
  effect_in.addKeyframe(keyframe);
  Keyframe keyframeBits(bl + 1, (float)BITS_EFFECT, 0);
  effect_in.addKeyframe(keyframeBits);
  testingBand2.addEffect(effect_in);

  testingBand0.addEffect(testingEffect0);
  // testingBand2.addEffect(testingEffect2);
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

  SECTION("MetadataExperience") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writeUnits(testingHaptic, bitstream, PACKET_DURATION);
    haptics::types::Haptics readHaptic;
    IOStream::StreamReader buffer = IOStream::initializeStream();
    IOStream::CRC crc;
    for (auto &packetBits : bitstream) {
      succeed &= IOStream::readMIHSUnit(packetBits, buffer, crc);
      // buffer.time += buffer.packetDuration;
    }
    REQUIRE(succeed);
    readHaptic = buffer.haptic;
    // Check Haptic Experience information
    CHECK(readHaptic.getVersion() == testingHaptic.getVersion());
    CHECK(readHaptic.getDate() == testingHaptic.getDate());
    CHECK(readHaptic.getDescription() == testingHaptic.getDescription());
    CHECK(readHaptic.getPerceptionsSize() == testingHaptic.getPerceptionsSize());
    CHECK(readHaptic.getAvatarsSize() == testingHaptic.getAvatarsSize());

    // Check Avatar information
    for (auto i = 0; i < static_cast<int>(readHaptic.getAvatarsSize()); i++) {
      for (auto j = 0; j < static_cast<int>(testingHaptic.getAvatarsSize()); j++) {
        if (readHaptic.getAvatarAt(i).getId() == testingHaptic.getAvatarAt(j).getId()) {
          CHECK(readHaptic.getAvatarAt(i).getLod() == testingHaptic.getAvatarAt(j).getLod());
          CHECK(readHaptic.getAvatarAt(i).getType() == testingHaptic.getAvatarAt(j).getType());
          if (readHaptic.getAvatarAt(i).getType() == haptics::types::AvatarType::Custom) {
            std::string readHapticMesh = readHaptic.getAvatarAt(i).getMesh().value();
            std::string testingHapticMesh = testingHaptic.getAvatarAt(j).getMesh().value();
            CHECK(readHapticMesh == testingHapticMesh);
          }
        }
      }
    }
  }
  SECTION("Read MetadataPerception") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writeUnits(testingHaptic, bitstream, PACKET_DURATION);
    haptics::types::Haptics readHaptic;
    IOStream::StreamReader buffer = IOStream::initializeStream();
    IOStream::CRC crc;
    for (auto &packetBits : bitstream) {
      succeed &= IOStream::readMIHSUnit(packetBits, buffer, crc);
    }

    REQUIRE(succeed);

    readHaptic = buffer.haptic;
    // Check Haptic Perception information
    haptics::types::Perception readPerception0 = readHaptic.getPerceptionAt(0);
    CHECK(readPerception0.getId() == testingPerception0.getId());
    CHECK(readPerception0.getDescription() == testingPerception0.getDescription());
    CHECK(readPerception0.getPerceptionModality() == testingPerception0.getPerceptionModality());
    CHECK(readPerception0.getAvatarId() == testingPerception0.getAvatarId());
    CHECK(readPerception0.getEffectLibrarySize() == testingPerception0.getEffectLibrarySize());
    CHECK(readPerception0.getUnitExponent() == testingPerception0.getUnitExponentOrDefault());
    CHECK(readPerception0.getPerceptionUnitExponent() ==
          testingPerception0.getPerceptionUnitExponentOrDefault());
    CHECK(readPerception0.getChannelsSize() == testingPerception0.getChannelsSize());
    CHECK(readPerception0.getReferenceDevicesSize() ==
          testingPerception0.getReferenceDevicesSize());
    if (readPerception0.getReferenceDevicesSize() > 0) {
      for (auto i = 0; i < static_cast<int>(readPerception0.getReferenceDevicesSize()); i++) {
        haptics::types::ReferenceDevice readRefDev = readPerception0.getReferenceDeviceAt(i);
        for (auto j = 0; j < static_cast<int>(testingPerception0.getReferenceDevicesSize()); j++) {
          haptics::types::ReferenceDevice testingRefDev =
              testingPerception0.getReferenceDeviceAt(j);
          if (readRefDev.getId() == testingRefDev.getId()) {
            CHECK(readRefDev.getName() == testingRefDev.getName());
            CHECK(readRefDev.getBodyPartMask().value() ==
                  testingRefDev.getBodyPartMask().value_or(0));

            if (readRefDev.getMaximumFrequency().has_value()) {
              CHECK(readRefDev.getMaximumFrequency().value() -
                        testingRefDev.getMaximumFrequency().value() <
                    floatPrecision);
            }
            if (readRefDev.getMinimumFrequency().has_value()) {
              CHECK(readRefDev.getMinimumFrequency().value() -
                        testingRefDev.getMinimumFrequency().value() <
                    floatPrecision);
            }
            if (readRefDev.getResonanceFrequency().has_value()) {
              CHECK(readRefDev.getResonanceFrequency().value() -
                        testingRefDev.getResonanceFrequency().value() <
                    floatPrecision);
            }
            if (readRefDev.getMaximumAmplitude().has_value()) {
              CHECK(readRefDev.getMaximumAmplitude().value() -
                        testingRefDev.getMaximumAmplitude().value() <
                    floatPrecision);
            }
            if (readRefDev.getImpedance().has_value()) {
              CHECK(readRefDev.getImpedance().value() - testingRefDev.getImpedance().value() <
                    floatPrecision);
            }
            if (readRefDev.getMaximumVoltage().has_value()) {
              CHECK(readRefDev.getMaximumVoltage().value() -
                        testingRefDev.getMaximumVoltage().value() <
                    floatPrecision);
            }
            if (readRefDev.getMaximumCurrent().has_value()) {
              CHECK(readRefDev.getMaximumCurrent().value() -
                        testingRefDev.getMaximumCurrent().value() <
                    floatPrecision);
            }
            if (readRefDev.getMaximumDisplacement().has_value()) {
              CHECK(readRefDev.getMaximumDisplacement().value() -
                        testingRefDev.getMaximumDisplacement().value() <
                    floatPrecision);
            }
            if (readRefDev.getWeight().has_value()) {
              CHECK(readRefDev.getWeight().value() - testingRefDev.getWeight().value() <
                    floatPrecision);
            }
            if (readRefDev.getSize().has_value()) {
              CHECK(readRefDev.getSize().value() - testingRefDev.getSize().value() <
                    floatPrecision);
            }
            if (readRefDev.getCustom().has_value()) {
              CHECK(readRefDev.getCustom().value() - testingRefDev.getCustom().value() <
                    floatPrecision);
            }
            if (readRefDev.getType().has_value()) {
              CHECK(readRefDev.getType().value() == testingRefDev.getType().value());
            }
          }
        }
      }
    }
  }
  SECTION("MetadataChannel") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writeUnits(testingHaptic, bitstream, PACKET_DURATION);
    haptics::types::Haptics readHaptic;
    IOStream::StreamReader buffer = IOStream::initializeStream();
    IOStream::CRC crc;
    for (auto &packetBits : bitstream) {
      succeed &= IOStream::readMIHSUnit(packetBits, buffer, crc);
    }

    REQUIRE(succeed);

    readHaptic = buffer.haptic;
    // Check Haptic Perception information
    haptics::types::Channel readChannel0 = readHaptic.getPerceptionAt(0).getChannelAt(0);
    CHECK(readChannel0.getId() == testingChannel0.getId());
    CHECK(readChannel0.getDescription() == testingChannel0.getDescription());
    if (readChannel0.getReferenceDeviceId().has_value() &&
        testingChannel0.getReferenceDeviceId().has_value()) {
      CHECK(readChannel0.getReferenceDeviceId().value() ==
            testingChannel0.getReferenceDeviceId().value());
    }
    CHECK(readChannel0.getGain() - testingChannel0.getGain() < floatPrecision);
    CHECK(readChannel0.getMixingWeight() - testingChannel0.getMixingWeight() < floatPrecision);
    CHECK(readChannel0.getBodyPartMask() == testingChannel0.getBodyPartMask());
    CHECK(readChannel0.getFrequencySampling() == testingChannel0.getFrequencySampling());
    if (readChannel0.getFrequencySampling() > 0) {
      CHECK(readChannel0.getSampleCount() == testingChannel0.getSampleCount());
    }
    CHECK(readChannel0.getDirection().has_value() == testingChannel0.getDirection().has_value());
    if (readChannel0.getDirection().has_value()) {
      CHECK(readChannel0.getDirection().value().X == testingChannel0.getDirection().value().X);
      CHECK(readChannel0.getDirection().value().Y == testingChannel0.getDirection().value().Y);
      CHECK(readChannel0.getDirection().value().Z == testingChannel0.getDirection().value().Z);
    }
    CHECK(readChannel0.getVerticesSize() == testingChannel0.getVerticesSize());
    if (readChannel0.getVerticesSize() > 0) {
      for (size_t i = 0; i < readChannel0.getVerticesSize(); i++) {
        CHECK(readChannel0.getVertexAt(i) - testingChannel0.getVertexAt(i) < floatPrecision);
      }
    }

    haptics::types::Channel readChannel1 = readHaptic.getPerceptionAt(1).getChannelAt(0);
    CHECK(readChannel1.getId() == testingChannel2.getId());
    CHECK(readChannel1.getDescription() == testingChannel2.getDescription());
    if (readChannel1.getReferenceDeviceId().has_value()) {
      CHECK(readChannel1.getReferenceDeviceId().value() ==
            testingChannel1.getReferenceDeviceId().value());
    }
    CHECK(readChannel1.getGain() - testingChannel2.getGain() < floatPrecision);
    CHECK(readChannel1.getMixingWeight() - testingChannel2.getMixingWeight() < floatPrecision);
    CHECK(readChannel1.getBodyPartMask() == testingChannel2.getBodyPartMask());
    CHECK(readChannel1.getFrequencySampling() == testingChannel2.getFrequencySampling());
    if (readChannel1.getFrequencySampling() > 0) {
      CHECK(readChannel1.getSampleCount() == testingChannel2.getSampleCount());
    }
    CHECK(readChannel1.getDirection().has_value() == testingChannel2.getDirection().has_value());
    if (readChannel1.getDirection().has_value()) {
      CHECK(readChannel1.getDirection().value().X == testingChannel2.getDirection().value().X);
      CHECK(readChannel1.getDirection().value().Y == testingChannel2.getDirection().value().Y);
      CHECK(readChannel1.getDirection().value().Z == testingChannel2.getDirection().value().Z);
    }
    CHECK(readChannel1.getVerticesSize() == testingChannel2.getVerticesSize());
    if (readChannel1.getVerticesSize() > 0) {
      for (size_t i = 0; i < readChannel1.getVerticesSize(); i++) {
        CHECK(readChannel1.getVertexAt(i) - testingChannel2.getVertexAt(i) < floatPrecision);
      }
    }
  }

  SECTION("MetadataBand") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writeUnits(testingHaptic, bitstream, PACKET_DURATION);
    haptics::types::Haptics readHaptic;
    IOStream::StreamReader buffer = IOStream::initializeStream();
    IOStream::CRC crc;
    for (auto &packetBits : bitstream) {
      succeed &= IOStream::readMIHSUnit(packetBits, buffer, crc);
    }
    REQUIRE(succeed);

    readHaptic = buffer.haptic;
    // Check Haptic Band information
    haptics::types::Band readBand0 = readHaptic.getPerceptionAt(0).getChannelAt(1).getBandAt(0);
    CHECK(readBand0.getBandType() == testingBand1.getBandType());
    CHECK(readBand0.getCurveType() == testingBand1.getCurveType());
    CHECK(readBand0.getLowerFrequencyLimit() == testingBand1.getLowerFrequencyLimit());
    CHECK(readBand0.getUpperFrequencyLimit() == testingBand1.getUpperFrequencyLimit());
    CHECK(readBand0.getEffectsSize() == testingBand1.getEffectsSize());

    haptics::types::Band readBand1 = readHaptic.getPerceptionAt(0).getChannelAt(0).getBandAt(0);
    CHECK(readBand1.getBandType() == testingBand0.getBandType());
    // CHECK(readBand2.getCurveType() == testingBand2.getCurveType());
    CHECK(readBand1.getLowerFrequencyLimit() == testingBand0.getLowerFrequencyLimit());
    CHECK(readBand1.getUpperFrequencyLimit() == testingBand0.getUpperFrequencyLimit());
    CHECK(readBand1.getEffectsSize() == testingBand0.getEffectsSize());
  }

  SECTION("Databand packets") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writeUnits(testingHaptic, bitstream, PACKET_DURATION);
    haptics::types::Haptics readHaptic;
    IOStream::StreamReader buffer = IOStream::initializeStream();
    IOStream::CRC crc;
    for (auto &packetBits : bitstream) {
      succeed &= IOStream::readMIHSUnit(packetBits, buffer, crc);
    }
    REQUIRE(succeed);
    CHECK(bitstream.size() == PACKET_NUMBER);

    // CHECK metadata experience length is correct
    REQUIRE(succeed);
    readHaptic = buffer.haptic;

    CHECK(readHaptic.getPerceptionsSize() == testingHaptic.getPerceptionsSize());
  }

  SECTION("Save/Read binary streaming file") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writeUnits(testingHaptic, bitstream, PACKET_DURATION);
    std::string filepath = "test.impg";
    IOStream::writeFile(testingHaptic, filepath, PACKET_DURATION);

    std::vector<std::vector<bool>> readBitstream = std::vector<std::vector<bool>>();
    IOStream::loadFile(filepath, readBitstream);

    haptics::types::Haptics readHaptic;
    IOStream::readFile(filepath, readHaptic);

    REQUIRE(succeed);
  }
}