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
constexpr int CONST_8 = 8;
constexpr size_t bl = 512;
constexpr size_t level = 7;
constexpr int BITS_EFFECT = 15;
constexpr int MOD_VAL = 256;
constexpr float scalar = 1.5;

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

  const int testingId_perception1 = 423;
  const int testingAvatarId_perception1 = 3;
  const std::string testingDescription_perception1 = "This developer need an HAPTIC coffee !";
  const auto testingPerceptionModality_perception1 = haptics::types::PerceptionModality::Other;
  haptics::types::Perception testingPerception1(testingId_perception1, testingAvatarId_perception1,
                                                testingDescription_perception1,
                                                testingPerceptionModality_perception1);

  const int testingId_track0 = 0;
  const std::string testingDescription_track0 = "testingDescription_track0";
  const float testingGain_track0 = .34;
  const float testingMixingWeight_track0 = 1;
  const uint32_t testingBodyPartMask_track0 = 32;
  const std::vector<int> testingVertices_track0 = {0, 453, -3, 7657};
  haptics::types::Track testingTrack0(testingId_track0, testingDescription_track0,
                                      testingGain_track0, testingMixingWeight_track0,
                                      testingBodyPartMask_track0);
  for (auto vertex : testingVertices_track0) {
    testingTrack0.addVertex(vertex);
  }

  const int testingId_track1 = 432;
  const std::string testingDescription_track1 = "again another string";
  const float testingGain_track1 = 0;
  const float testingMixingWeight_track1 = .333;
  const uint32_t testingBodyPartMask_track1 = ~(uint32_t)(0);
  haptics::types::Track testingTrack1(testingId_track1, testingDescription_track1,
                                      testingGain_track1, testingMixingWeight_track1,
                                      testingBodyPartMask_track1);

  const int testingId_track2 = 4;
  const std::string testingDescription_track2 = "I'm inside a test";
  const float testingGain_track2 = 2.7652;
  const float testingMixingWeight_track2 = .6666;
  const uint32_t testingBodyPartMask_track2 = 0;
  const std::vector<int> testingVertices_track2 = {0, 6};
  haptics::types::Track testingTrack2(testingId_track2, testingDescription_track2,
                                      testingGain_track2, testingMixingWeight_track2,
                                      testingBodyPartMask_track2);
  for (auto vertex : testingVertices_track2) {
    testingTrack2.addVertex(vertex);
  }

  const auto testingBandType_band0 = haptics::types::BandType::Curve;
  const auto testingCurveType_band0 = haptics::types::CurveType::Cubic;
  const int testingWindowLength_band0 = 0;
  const int testingLowerFrequencyLimit_band0 = 0;
  const int testingUpperFrequencyLimit_band0 = 75;
  haptics::types::Band testingBand0(testingBandType_band0, testingCurveType_band0,
                                    testingWindowLength_band0, testingLowerFrequencyLimit_band0,
                                    testingUpperFrequencyLimit_band0);

  const auto testingBandType_band1 = haptics::types::BandType::Transient;
  const auto testingCurveType_band1 = haptics::types::CurveType::Unknown;
  const int testingWindowLength_band1 = 0;
  const int testingLowerFrequencyLimit_band1 = 65;
  const int testingUpperFrequencyLimit_band1 = 300;
  haptics::types::Band testingBand1(testingBandType_band1, testingCurveType_band1,
                                    testingWindowLength_band1, testingLowerFrequencyLimit_band1,
                                    testingUpperFrequencyLimit_band1);

  const auto testingBandType_band2 = haptics::types::BandType::WaveletWave;
  const auto testingCurveType_band2 = haptics::types::CurveType::Unknown;
  const int testingWindowLength_band2 = 128;
  const int testingLowerFrequencyLimit_band2 = 0;
  const int testingUpperFrequencyLimit_band2 = 1000;
  haptics::types::Band testingBand2(testingBandType_band2, testingCurveType_band2,
                                    testingWindowLength_band2, testingLowerFrequencyLimit_band2,
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
  testingBand2.addEffect(testingEffect2);
  testingTrack0.addBand(testingBand0);
  testingTrack1.addBand(testingBand1);
  testingTrack2.addBand(testingBand2);
  testingPerception0.addTrack(testingTrack0);
  testingPerception0.addTrack(testingTrack1);
  testingPerception1.addTrack(testingTrack2);
  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);
  testingHaptic.addAvatar(avatar1);
  testingHaptic.addAvatar(avatar2);
  SECTION("Write metadataExperience") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);

    REQUIRE(succeed);
    CHECK(bitstream.size() == 12);
    const uintmax_t sizeAvatar1 = haptics::io::AVATAR_ID + haptics::io::AVATAR_LOD +
                                  haptics::io::AVATAR_TYPE + haptics::io::AVATAR_MESH_COUNT +
                                  (avatar1.getMesh().value().size() * haptics::io::BYTE_SIZE);

    const uintmax_t sizeAvatar2 =
        haptics::io::AVATAR_ID + haptics::io::AVATAR_LOD + haptics::io::AVATAR_TYPE;

    const uintmax_t sizeMetaExperience =
        haptics::io::MDEXP_VERSION + (haptics::io::BYTE_SIZE * testingHaptic.getVersion().size()) +
        haptics::io::MDEXP_DATE + (haptics::io::BYTE_SIZE * testingHaptic.getDate().size()) +
        haptics::io::MDEXP_DESC_SIZE +
        (haptics::io::BYTE_SIZE * testingHaptic.getDescription().size()) +
        haptics::io::MDEXP_PERC_COUNT + haptics::io::MDEXP_AVATAR_COUNT + sizeAvatar1 + sizeAvatar2;
    uintmax_t sizeMetaExpBytes =
        (CONST_8 - (sizeMetaExperience % CONST_8) + sizeMetaExperience) / CONST_8;

    std::vector<bool> metaExpPacket = bitstream[0];
    int beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    std::vector<bool> packetLength(metaExpPacket.begin() + beginIdx,
                                   metaExpPacket.begin() + beginIdx +
                                       haptics::io::H_PAYLOAD_LENGTH);
    std::string packetLengthStr;
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaExpLen =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaExpLen == sizeMetaExpBytes);
  }

  SECTION("Read metadataExperience") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeStream(readHaptic);

    for (auto &packetBits : bitstream) {

      succeed &= IOStream::readNALu(readHaptic, packetBits, buffer);
    }
    REQUIRE(succeed);
    // Check Haptic Experience information
    CHECK(readHaptic.getVersion() == testingHaptic.getVersion());
    CHECK(readHaptic.getDate() == testingHaptic.getDate());
    CHECK(readHaptic.getDescription() == testingHaptic.getDescription());
    CHECK(readHaptic.getPerceptionsSize() == testingHaptic.getPerceptionsSize());
    CHECK(readHaptic.getAvatarsSize() == testingHaptic.getAvatarsSize());

    // Check Avatar information
    for (int i = 0; i < readHaptic.getAvatarsSize(); i++) {
      for (int j = 0; j < testingHaptic.getAvatarsSize(); j++) {
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

  SECTION("Write metadataPerception") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataPerception, testingHaptic, 0, bitstream);
    REQUIRE(succeed);
    CHECK(bitstream.size() == 1);
    const uintmax_t sizeMetaRefDev0 =
        haptics::io::REFDEV_ID + haptics::io::REFDEV_NAME_LENGTH +
        (haptics::io::BYTE_SIZE * testingPerception0.getReferenceDeviceAt(0).getName().size()) +
        haptics::io::REFDEV_BODY_PART_MASK + haptics::io::REFDEV_OPT_FIELDS +
        haptics::io::REFDEV_MAX_FREQ + haptics::io::REFDEV_MIN_FREQ + haptics::io::REFDEV_MAX_AMP +
        haptics::io::REFDEV_CUSTOM + haptics::io::REFDEV_TYPE;

    const uintmax_t sizeMetaRefDev1 =
        haptics::io::REFDEV_ID + haptics::io::REFDEV_NAME_LENGTH +
        (haptics::io::BYTE_SIZE * testingPerception0.getReferenceDeviceAt(1).getName().size()) +
        haptics::io::REFDEV_BODY_PART_MASK + haptics::io::REFDEV_OPT_FIELDS +
        haptics::io::REFDEV_MAX_FREQ + haptics::io::REFDEV_MIN_FREQ + haptics::io::REFDEV_RES_FREQ +
        haptics::io::REFDEV_MAX_AMP + haptics::io::REFDEV_IMPEDANCE + haptics::io::REFDEV_MAX_VOLT +
        haptics::io::REFDEV_MAX_CURR + haptics::io::REFDEV_MAX_DISP + haptics::io::REFDEV_WEIGHT +
        haptics::io::REFDEV_SIZE + haptics::io::REFDEV_CUSTOM + haptics::io::REFDEV_TYPE;

    const uintmax_t sizeMetaRefDev2 =
        haptics::io::REFDEV_ID + haptics::io::REFDEV_NAME_LENGTH +
        (haptics::io::BYTE_SIZE * testingPerception0.getReferenceDeviceAt(2).getName().size()) +
        haptics::io::REFDEV_BODY_PART_MASK + haptics::io::REFDEV_OPT_FIELDS;

    const uintmax_t sizeMetaPerception0 =
        haptics::io::MDPERCE_ID + haptics::io::MDPERCE_DESC_SIZE +
        (haptics::io::BYTE_SIZE * testingPerception0.getDescription().size()) +
        haptics::io::MDPERCE_MODALITY + haptics::io::AVATAR_ID + haptics::io::MDPERCE_FXLIB_COUNT +
        haptics::io::MDPERCE_UNIT_EXP + haptics::io::MDPERCE_PERCE_UNIT_EXP +
        haptics::io::MDPERCE_REFDEVICE_COUNT + haptics::io::MDPERCE_TRACK_COUNT + sizeMetaRefDev0 +
        sizeMetaRefDev1 + sizeMetaRefDev2;
    uintmax_t sizeMetaPerceBytes =
        ((CONST_8 - (sizeMetaPerception0 % CONST_8)) + sizeMetaPerception0) / CONST_8;
    std::vector<bool> metaPerception0Packet = bitstream[0];
    int beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    std::vector<bool> packetLength(metaPerception0Packet.begin() + beginIdx,
                                   metaPerception0Packet.begin() + beginIdx +
                                       haptics::io::H_PAYLOAD_LENGTH);
    std::string packetLengthStr;
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaPerception0Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaPerception0Len == sizeMetaPerceBytes);
  }

  SECTION("Read metadataPerception") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeStream(readHaptic);

    for (auto &packetBits : bitstream) {
      succeed &= IOStream::readNALu(readHaptic, packetBits, buffer);
    }

    REQUIRE(succeed);
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
    CHECK(readPerception0.getTracksSize() == testingPerception0.getTracksSize());
    CHECK(readPerception0.getReferenceDevicesSize() ==
          testingPerception0.getReferenceDevicesSize());
    if (readPerception0.getReferenceDevicesSize() > 0) {
      for (int i = 0; i < readPerception0.getReferenceDevicesSize(); i++) {
        haptics::types::ReferenceDevice readRefDev = readPerception0.getReferenceDeviceAt(i);
        for (int j = 0; j < testingPerception0.getReferenceDevicesSize(); j++) {
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

  SECTION("Write metadataTrack") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataTrack, testingHaptic, 0, bitstream);
    REQUIRE(succeed);
    CHECK(bitstream.size() == 2);
    const uintmax_t sizeMetaTrack0 =
        haptics::io::MDTRACK_ID + haptics::io::MDTRACK_DESC_LENGTH +
        (haptics::io::BYTE_SIZE * testingTrack0.getDescription().size()) + haptics::io::REFDEV_ID +
        haptics::io::MDTRACK_GAIN + haptics::io::MDTRACK_MIXING_WEIGHT +
        haptics::io::MDTRACK_BODY_PART_MASK + haptics::io::MDTRACK_SAMPLING_FREQUENCY +
        haptics::io::MDTRACK_DIRECTION_MASK + haptics::io::MDTRACK_VERT_COUNT +
        (testingTrack0.getVerticesSize() * haptics::io::MDTRACK_VERT) + haptics::io::MDBAND_ID;

    uintmax_t sizeMetaTrack0Bytes =
        ((CONST_8 - (sizeMetaTrack0 % CONST_8)) + sizeMetaTrack0) / CONST_8;

    const uintmax_t sizeMetaTrack1 =
        haptics::io::MDTRACK_ID + haptics::io::MDTRACK_DESC_LENGTH +
        (haptics::io::BYTE_SIZE * testingTrack1.getDescription().size()) + haptics::io::REFDEV_ID +
        haptics::io::MDTRACK_GAIN + haptics::io::MDTRACK_MIXING_WEIGHT +
        haptics::io::MDTRACK_BODY_PART_MASK + haptics::io::MDTRACK_SAMPLING_FREQUENCY +
        haptics::io::MDTRACK_DIRECTION_MASK + haptics::io::MDTRACK_VERT_COUNT +
        haptics::io::MDBAND_ID;

    uintmax_t sizeMetaTrack1Bytes =
        ((CONST_8 - (sizeMetaTrack1 % CONST_8)) + sizeMetaTrack1) / CONST_8;

    std::vector<bool> metaTrack0Packet = bitstream[0];
    int beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    std::vector<bool> packetLength(metaTrack0Packet.begin() + beginIdx,
                                   metaTrack0Packet.begin() + beginIdx +
                                       haptics::io::H_PAYLOAD_LENGTH);
    std::string packetLengthStr;
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaTrack0Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaTrack0Len == sizeMetaTrack0Bytes);

    std::vector<bool> metaTrack1Packet = bitstream[1];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaTrack1Packet.begin() + beginIdx,
                          metaTrack1Packet.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaTrack1Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaTrack1Len == sizeMetaTrack1Bytes);
  }

  SECTION("Read metadataTrack") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);

    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeStream(readHaptic);

    for (auto &packetBits : bitstream) {

      succeed &= IOStream::readNALu(readHaptic, packetBits, buffer);
    }

    REQUIRE(succeed);
    // Check Haptic Perception information
    haptics::types::Track readTrack0 = readHaptic.getPerceptionAt(0).getTrackAt(1);
    CHECK(readTrack0.getId() == testingTrack0.getId());
    CHECK(readTrack0.getDescription() == testingTrack0.getDescription());
    if (readTrack0.getReferenceDeviceId().has_value()) {
      CHECK(readTrack0.getReferenceDeviceId() == testingTrack0.getReferenceDeviceId());
    }
    CHECK(readTrack0.getGain() - testingTrack0.getGain() < floatPrecision);
    CHECK(readTrack0.getMixingWeight() - testingTrack0.getMixingWeight() < floatPrecision);
    CHECK(readTrack0.getBodyPartMask() == testingTrack0.getBodyPartMask());
    CHECK(readTrack0.getFrequencySampling() == testingTrack0.getFrequencySampling());
    if (readTrack0.getFrequencySampling() > 0) {
      CHECK(readTrack0.getSampleCount() == testingTrack0.getSampleCount());
    }
    CHECK(readTrack0.getDirection().has_value() == testingTrack0.getDirection().has_value());
    if (readTrack0.getDirection().has_value()) {
      CHECK(readTrack0.getDirection().value().X == testingTrack0.getDirection().value().X);
      CHECK(readTrack0.getDirection().value().Y == testingTrack0.getDirection().value().Y);
      CHECK(readTrack0.getDirection().value().Z == testingTrack0.getDirection().value().Z);
    }
    CHECK(readTrack0.getVerticesSize() == testingTrack0.getVerticesSize());
    if (readTrack0.getVerticesSize() > 0) {
      for (int i = 0; i < readTrack0.getVerticesSize(); i++) {
        CHECK(readTrack0.getVertexAt(i) - testingTrack0.getVertexAt(i) < floatPrecision);
      }
    }

    haptics::types::Track readTrack1 = readHaptic.getPerceptionAt(0).getTrackAt(0);
    CHECK(readTrack1.getId() == testingTrack1.getId());
    CHECK(readTrack1.getDescription() == testingTrack1.getDescription());
    if (readTrack1.getReferenceDeviceId().has_value()) {
      CHECK(readTrack1.getReferenceDeviceId() == testingTrack1.getReferenceDeviceId());
    }
    CHECK(readTrack1.getGain() - testingTrack1.getGain() < floatPrecision);
    CHECK(readTrack1.getMixingWeight() - testingTrack1.getMixingWeight() < floatPrecision);
    CHECK(readTrack1.getBodyPartMask() == testingTrack1.getBodyPartMask());
    CHECK(readTrack1.getFrequencySampling() == testingTrack1.getFrequencySampling());
    if (readTrack1.getFrequencySampling() > 0) {
      CHECK(readTrack1.getSampleCount() == testingTrack1.getSampleCount());
    }
    CHECK(readTrack1.getDirection().has_value() == testingTrack1.getDirection().has_value());
    if (readTrack1.getDirection().has_value()) {
      CHECK(readTrack1.getDirection().value().X == testingTrack1.getDirection().value().X);
      CHECK(readTrack1.getDirection().value().Y == testingTrack1.getDirection().value().Y);
      CHECK(readTrack1.getDirection().value().Z == testingTrack1.getDirection().value().Z);
    }
    CHECK(readTrack1.getVerticesSize() == testingTrack1.getVerticesSize());
    if (readTrack1.getVerticesSize() > 0) {
      for (int i = 0; i < readTrack1.getVerticesSize(); i++) {
        CHECK(readTrack1.getVertexAt(i) - testingTrack1.getVertexAt(i) < floatPrecision);
      }
    }
  }

  SECTION("Write metadataBand") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataBand, testingHaptic, 0, bitstream);
    REQUIRE(succeed);
    CHECK(bitstream.size() == 2);
    const uintmax_t sizeMetaBand0 = haptics::io::MDBAND_ID + haptics::io::MDBAND_BAND_TYPE +
                                    haptics::io::MDBAND_CURVE_TYPE + haptics::io::MDBAND_LOW_FREQ +
                                    haptics::io::MDBAND_UP_FREQ + haptics::io::MDBAND_FX_COUNT;

    const uintmax_t sizeMetaBand2 = haptics::io::MDBAND_ID + haptics::io::MDBAND_BAND_TYPE +
                                    haptics::io::MDBAND_LOW_FREQ + haptics::io::MDBAND_UP_FREQ +
                                    haptics::io::MDBAND_FX_COUNT;

    uintmax_t sizeMetaBand0Bytes =
        ((CONST_8 - (sizeMetaBand0 % CONST_8)) + sizeMetaBand0) / CONST_8;
    uintmax_t sizeMetaBand2Bytes =
        ((CONST_8 - (sizeMetaBand2 % CONST_8)) + sizeMetaBand2) / CONST_8;

    std::vector<bool> metaBand0Packet = bitstream[0];
    int beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    std::vector<bool> packetLength(metaBand0Packet.begin() + beginIdx,
                                   metaBand0Packet.begin() + beginIdx +
                                       haptics::io::H_PAYLOAD_LENGTH);
    std::string packetLengthStr;
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaBand0Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaBand0Len == sizeMetaBand0Bytes);

    std::vector<bool> metaBand1Packet = bitstream[1];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaBand1Packet.begin() + beginIdx,
                          metaBand1Packet.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaBand1Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaBand1Len == sizeMetaBand2Bytes);
  }

  SECTION("Read metadataBand") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeStream(readHaptic);

    for (auto &packetBits : bitstream) {

      succeed &= IOStream::readNALu(readHaptic, packetBits, buffer);
    }
    REQUIRE(succeed);
    // Check Haptic Band information
    haptics::types::Band readBand0 = readHaptic.getPerceptionAt(0).getTrackAt(1).getBandAt(0);
    CHECK(readBand0.getBandType() == testingBand0.getBandType());
    CHECK(readBand0.getCurveType() == testingBand0.getCurveType());
    CHECK(readBand0.getLowerFrequencyLimit() == testingBand0.getLowerFrequencyLimit());
    CHECK(readBand0.getUpperFrequencyLimit() == testingBand0.getUpperFrequencyLimit());
    CHECK(readBand0.getEffectsSize() == testingBand0.getEffectsSize());

    haptics::types::Band readBand1 = readHaptic.getPerceptionAt(0).getTrackAt(0).getBandAt(0);
    CHECK(readBand1.getBandType() == testingBand1.getBandType());
    // CHECK(readBand2.getCurveType() == testingBand2.getCurveType());
    CHECK(readBand1.getLowerFrequencyLimit() == testingBand1.getLowerFrequencyLimit());
    CHECK(readBand1.getUpperFrequencyLimit() == testingBand1.getUpperFrequencyLimit());
    CHECK(readBand1.getEffectsSize() == testingBand1.getEffectsSize());
  }

  SECTION("Write/Read Databand packets") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);

    REQUIRE(succeed);
    CHECK(bitstream.size() == 12);

    // CHECK metadata experience length is correct
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeStream(readHaptic);

    for (auto &packetBits : bitstream) {

      succeed &= IOStream::readNALu(readHaptic, packetBits, buffer);
    }

    REQUIRE(succeed);

    CHECK(readHaptic.getPerceptionsSize() == testingHaptic.getPerceptionsSize());
  }

  SECTION("Save/Read binary streaming file") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    std::string filepath = "test.impg";
    IOStream::writeFile(testingHaptic, filepath);

    std::vector<std::vector<bool>> readBitstream = std::vector<std::vector<bool>>();
    IOStream::loadFile(filepath, readBitstream);

    haptics::types::Haptics readHaptic;
    IOStream::readFile(filepath, readHaptic);

    REQUIRE(succeed);
    CHECK(bitstream == readBitstream);
  }
  // SECTION("Read packets") {}
};