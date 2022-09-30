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

TEST_CASE("Write/read MetadataExperience packet") {
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

  const int testingId_perception1 = 423;
  const int testingAvatarId_perception1 = 3;
  const std::string testingDescription_perception1 = "This developer need an HAPTIC coffee !";
  const auto testingPerceptionModality_perception1 = haptics::types::PerceptionModality::Other;
  haptics::types::Perception testingPerception1(testingId_perception1, testingAvatarId_perception1,
                                                testingDescription_perception1,
                                                testingPerceptionModality_perception1);

  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);
  testingHaptic.addAvatar(avatar1);
  testingHaptic.addAvatar(avatar2);

  SECTION("Write metadataExperience") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);

    REQUIRE(succeed);
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
    CHECK(metaExpLen == sizeMetaExperience);
  }

  SECTION("Read metadataExperience") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataHaptics, testingHaptic, 0, bitstream);
    std::vector<bool> metaExpPacket = bitstream[0];
    haptics::types::Haptics readHaptic = haptics::types::Haptics();
    succeed = IOStream::readPacket(readHaptic, metaExpPacket);
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
}

TEST_CASE("Write/read MetadataPerception packet") {
  const std::string testingVersion = "RM1";
  const std::string testingDate = "Today";
  const std::string testingDescription = "I'm a testing value";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

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
           std::nullopt, std::nullopt, std::nullopt, std::nullopt, 24.42,
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

  testingPerception0.addTrack(testingTrack0);
  testingPerception0.addTrack(testingTrack1);
  testingPerception1.addTrack(testingTrack2);
  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);

  SECTION("Write metadataPerception") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataPerception, testingHaptic, 0, bitstream);
    REQUIRE(succeed);
    CHECK(bitstream.size() == 2);
    const uintmax_t sizeMetaRefDev0 =
        haptics::io::REFDEV_ID + haptics::io::REFDEV_NAME_LENGTH +
        (haptics::io::BYTE_SIZE * testingPerception0.getReferenceDeviceAt(0).getName().size()) +
        haptics::io::REFDEV_OPT_FIELDS + haptics::io::REFDEV_MAX_FREQ +
        haptics::io::REFDEV_MIN_FREQ + haptics::io::REFDEV_MAX_AMP + haptics::io::REFDEV_CUSTOM +
        haptics::io::REFDEV_TYPE;

    const uintmax_t sizeMetaRefDev1 =
        haptics::io::REFDEV_ID + haptics::io::REFDEV_NAME_LENGTH +
        (haptics::io::BYTE_SIZE * testingPerception0.getReferenceDeviceAt(1).getName().size()) +
        haptics::io::REFDEV_OPT_FIELDS + haptics::io::REFDEV_BODY_PART_MASK +
        haptics::io::REFDEV_MAX_FREQ + haptics::io::REFDEV_MIN_FREQ + haptics::io::REFDEV_RES_FREQ +
        haptics::io::REFDEV_MAX_AMP + haptics::io::REFDEV_IMPEDANCE + haptics::io::REFDEV_MAX_VOLT +
        haptics::io::REFDEV_MAX_CURR + haptics::io::REFDEV_MAX_DISP + haptics::io::REFDEV_WEIGHT +
        haptics::io::REFDEV_SIZE + haptics::io::REFDEV_CUSTOM + haptics::io::REFDEV_TYPE;

    const uintmax_t sizeMetaRefDev2 =
        haptics::io::REFDEV_ID + haptics::io::REFDEV_NAME_LENGTH +
        (haptics::io::BYTE_SIZE * testingPerception0.getReferenceDeviceAt(2).getName().size()) +
        haptics::io::REFDEV_OPT_FIELDS;

    const uintmax_t sizeMetaPerception0 =
        haptics::io::MDPERCE_ID + haptics::io::MDPERCE_DESC_SIZE +
        (haptics::io::BYTE_SIZE * testingPerception0.getDescription().size()) +
        haptics::io::MDPERCE_MODALITY + haptics::io::AVATAR_ID + haptics::io::MDPERCE_FXLIB_COUNT +
        haptics::io::MDPERCE_UNIT_EXP + haptics::io::MDPERCE_PERCE_UNIT_EXP +
        haptics::io::MDPERCE_REFDEVICE_COUNT + haptics::io::MDPERCE_TRACK_COUNT + sizeMetaRefDev0 +
        sizeMetaRefDev1 + sizeMetaRefDev2;

    const uintmax_t sizeMetaPerception1 =
        haptics::io::MDPERCE_ID + haptics::io::MDPERCE_DESC_SIZE +
        (haptics::io::BYTE_SIZE * testingPerception1.getDescription().size()) +
        haptics::io::MDPERCE_MODALITY + haptics::io::AVATAR_ID + haptics::io::MDPERCE_FXLIB_COUNT +
        haptics::io::MDPERCE_UNIT_EXP + haptics::io::MDPERCE_PERCE_UNIT_EXP +
        haptics::io::MDPERCE_REFDEVICE_COUNT + haptics::io::MDPERCE_TRACK_COUNT;

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
    CHECK(metaPerception0Len == sizeMetaPerception0);

    std::vector<bool> metaPerception1Packet = bitstream[1];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaPerception1Packet.begin() + beginIdx,
                          metaPerception1Packet.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaPerception1Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaPerception1Len == sizeMetaPerception1);
  }

  SECTION("Read metadataPerception") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    std::vector<bool> metaHaptic = bitstream[0];
    std::vector<bool> metaPerception0 = bitstream[1];
    std::vector<bool> metaPerception1 = bitstream[2];
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeBuffer(readHaptic);
    succeed = IOStream::readNALu(readHaptic, metaHaptic, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaPerception0, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaPerception1, buffer);

    REQUIRE(succeed);
    // Check Haptic Perception information
    haptics::types::Perception readPerception0 = buffer.perceptionsBuffer[0];
    CHECK(readPerception0.getId() == testingPerception0.getId());
    CHECK(readPerception0.getDescription() == testingPerception0.getDescription());
    CHECK(readPerception0.getPerceptionModality() == testingPerception0.getPerceptionModality());
    CHECK(readPerception0.getAvatarId() == testingPerception0.getAvatarId());
    CHECK(readPerception0.getEffectLibrarySize() == testingPerception0.getEffectLibrarySize());
    CHECK(readPerception0.getUnitExponent() == testingPerception0.getUnitExponent().value_or(0));
    CHECK(readPerception0.getPerceptionUnitExponent() ==
          testingPerception0.getPerceptionUnitExponent().value_or(1));
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
            if (readRefDev.getBodyPartMask().has_value()) {
              CHECK(readRefDev.getBodyPartMask().value() ==
                    testingRefDev.getBodyPartMask().value());
            }
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

    haptics::types::Perception readPerception1 = buffer.perceptionsBuffer[1];
    CHECK(readPerception1.getId() == testingPerception1.getId());
    CHECK(readPerception1.getDescription() == testingPerception1.getDescription());
    CHECK(readPerception1.getPerceptionModality() == testingPerception1.getPerceptionModality());
    CHECK(readPerception1.getAvatarId() == testingPerception1.getAvatarId());
    CHECK(readPerception1.getEffectLibrarySize() == testingPerception1.getEffectLibrarySize());
    CHECK(readPerception1.getUnitExponent() == testingPerception1.getUnitExponent().value_or(0));
    CHECK(readPerception1.getPerceptionUnitExponent() ==
          testingPerception1.getPerceptionUnitExponent().value_or(1));
    CHECK(readPerception1.getTracksSize() == testingPerception1.getTracksSize());
    CHECK(readPerception1.getReferenceDevicesSize() ==
          testingPerception1.getReferenceDevicesSize());
    if (readPerception1.getReferenceDevicesSize() > 0) {
      for (int i = 0; i < readPerception1.getReferenceDevicesSize(); i++) {
        haptics::types::ReferenceDevice readRefDev = readPerception1.getReferenceDeviceAt(i);
        for (int j = 0; j < testingPerception1.getReferenceDevicesSize(); j++) {
          haptics::types::ReferenceDevice testingRefDev =
              testingPerception1.getReferenceDeviceAt(j);
          if (readRefDev.getId() == testingRefDev.getId()) {
            CHECK(readRefDev.getName() == testingRefDev.getName());
            if (readRefDev.getBodyPartMask().has_value()) {
              CHECK(readRefDev.getBodyPartMask().value() ==
                    testingRefDev.getBodyPartMask().value());
            }
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
};

TEST_CASE("Write/read MetadataTrack packet") {
  const std::string testingVersion = "RM1";
  const std::string testingDate = "Today";
  const std::string testingDescription = "I'm a testing value";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

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

  testingTrack0.addBand(haptics::types::Band());
  testingTrack0.addBand(haptics::types::Band());
  testingTrack0.addBand(haptics::types::Band());
  testingTrack1.addBand(haptics::types::Band());
  testingPerception0.addTrack(testingTrack0);
  testingPerception0.addTrack(testingTrack1);
  testingPerception1.addTrack(testingTrack2);
  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);

  SECTION("Write metadataTrack") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataTrack, testingHaptic, 0, bitstream);
    REQUIRE(succeed);
    CHECK(bitstream.size() == 3);
    const uintmax_t sizeMetaTrack0 =
        haptics::io::MDTRACK_ID + haptics::io::MDTRACK_DESC_LENGTH +
        (haptics::io::BYTE_SIZE * testingTrack0.getDescription().size()) + haptics::io::REFDEV_ID +
        haptics::io::MDTRACK_GAIN + haptics::io::MDTRACK_MIXING_WEIGHT +
        haptics::io::MDTRACK_BODY_PART_MASK + haptics::io::MDTRACK_SAMPLING_FREQUENCY +
        haptics::io::MDTRACK_DIRECTION_MASK + haptics::io::MDTRACK_VERT_COUNT +
        (testingTrack0.getVerticesSize() * haptics::io::MDTRACK_VERT) + haptics::io::MDBAND_ID;

    const uintmax_t sizeMetaTrack1 =
        haptics::io::MDTRACK_ID + haptics::io::MDTRACK_DESC_LENGTH +
        (haptics::io::BYTE_SIZE * testingTrack1.getDescription().size()) + haptics::io::REFDEV_ID +
        haptics::io::MDTRACK_GAIN + haptics::io::MDTRACK_MIXING_WEIGHT +
        haptics::io::MDTRACK_BODY_PART_MASK + haptics::io::MDTRACK_SAMPLING_FREQUENCY +
        haptics::io::MDTRACK_DIRECTION_MASK + haptics::io::MDTRACK_VERT_COUNT +
        haptics::io::MDBAND_ID;

    const uintmax_t sizeMetaTrack2 =
        haptics::io::MDTRACK_ID + haptics::io::MDTRACK_DESC_LENGTH +
        (haptics::io::BYTE_SIZE * testingTrack2.getDescription().size()) + haptics::io::REFDEV_ID +
        haptics::io::MDTRACK_GAIN + haptics::io::MDTRACK_MIXING_WEIGHT +
        haptics::io::MDTRACK_BODY_PART_MASK + haptics::io::MDTRACK_SAMPLING_FREQUENCY +
        haptics::io::MDTRACK_DIRECTION_MASK + haptics::io::MDTRACK_VERT_COUNT +
        (testingTrack2.getVerticesSize() * haptics::io::MDTRACK_VERT) + haptics::io::MDBAND_ID;

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
    CHECK(metaTrack0Len == sizeMetaTrack0);

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
    CHECK(metaTrack1Len == sizeMetaTrack1);

    std::vector<bool> metaTrack2Packet = bitstream[2];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaTrack2Packet.begin() + beginIdx,
                          metaTrack2Packet.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaTrack2Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaTrack2Len == sizeMetaTrack2);
  }

  SECTION("Read metadataTrack") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    std::vector<bool> metaHaptic = bitstream[0];
    std::vector<bool> metaPerception0 = bitstream[1];
    std::vector<bool> metaPerception1 = bitstream[2];
    std::vector<bool> metaTrack0 = bitstream[3];
    std::vector<bool> metaTrack1 = bitstream[4];
    std::vector<bool> metaTrack2 = bitstream[5];
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeBuffer(readHaptic);
    succeed = IOStream::readNALu(readHaptic, metaHaptic, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaTrack0, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaTrack1, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaTrack2, buffer);

    REQUIRE(succeed);
    // Check Haptic Perception information
    haptics::types::Track readTrack0 = buffer.tracksBuffer[0];
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

    haptics::types::Track readTrack1 = buffer.tracksBuffer[1];
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

    haptics::types::Track readTrack2 = buffer.tracksBuffer[2];
    CHECK(readTrack2.getId() == testingTrack2.getId());
    CHECK(readTrack2.getDescription() == testingTrack2.getDescription());
    if (readTrack2.getReferenceDeviceId().has_value()) {
      CHECK(readTrack2.getReferenceDeviceId() == testingTrack2.getReferenceDeviceId());
    }
    CHECK(readTrack2.getGain() - testingTrack2.getGain() < floatPrecision);
    CHECK(readTrack2.getMixingWeight() - testingTrack2.getMixingWeight() < floatPrecision);
    CHECK(readTrack2.getBodyPartMask() == testingTrack2.getBodyPartMask());
    CHECK(readTrack2.getFrequencySampling() == testingTrack2.getFrequencySampling());
    if (readTrack2.getFrequencySampling() > 0) {
      CHECK(readTrack2.getSampleCount() == testingTrack2.getSampleCount());
    }
    CHECK(readTrack2.getDirection().has_value() == testingTrack2.getDirection().has_value());
    if (readTrack2.getDirection().has_value()) {
      CHECK(readTrack2.getDirection().value().X == testingTrack2.getDirection().value().X);
      CHECK(readTrack2.getDirection().value().Y == testingTrack2.getDirection().value().Y);
      CHECK(readTrack2.getDirection().value().Z == testingTrack2.getDirection().value().Z);
    }
    CHECK(readTrack2.getVerticesSize() == testingTrack2.getVerticesSize());
    if (readTrack2.getVerticesSize() > 0) {
      for (int i = 0; i < readTrack2.getVerticesSize(); i++) {
        CHECK(readTrack2.getVertexAt(i) - testingTrack2.getVertexAt(i) < floatPrecision);
      }
    }
  }
};

TEST_CASE("Write/read MetadataBand packet") {
  const std::string testingVersion = "RM1";
  const std::string testingDate = "Today";
  const std::string testingDescription = "I'm a testing value";
  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);

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

  const auto testingBandType_band2 = haptics::types::BandType::VectorialWave;
  const auto testingCurveType_band2 = haptics::types::CurveType::Unknown;
  const int testingWindowLength_band2 = 0;
  const int testingLowerFrequencyLimit_band2 = 0;
  const int testingUpperFrequencyLimit_band2 = 1000;
  haptics::types::Band testingBand2(testingBandType_band2, testingCurveType_band2,
                                    testingWindowLength_band2, testingLowerFrequencyLimit_band2,
                                    testingUpperFrequencyLimit_band2);

  testingTrack0.addBand(testingBand0);
  testingTrack1.addBand(testingBand1);
  testingTrack2.addBand(testingBand2);
  testingPerception0.addTrack(testingTrack0);
  testingPerception0.addTrack(testingTrack1);
  testingPerception1.addTrack(testingTrack2);
  testingHaptic.addPerception(testingPerception0);
  testingHaptic.addPerception(testingPerception1);

  SECTION("Write metadataBand") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed =
        IOStream::writeNALu(haptics::io::NALuType::MetadataBand, testingHaptic, 0, bitstream);
    REQUIRE(succeed);
    CHECK(bitstream.size() == 3);
    const uintmax_t sizeMetaBand = haptics::io::MDBAND_ID + haptics::io::MDBAND_BAND_TYPE +
                                   haptics::io::MDBAND_CURVE_TYPE + haptics::io::MDBAND_WIN_LEN +
                                   haptics::io::MDBAND_LOW_FREQ + haptics::io::MDBAND_UP_FREQ +
                                   haptics::io::MDBAND_FX_COUNT;

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
    CHECK(metaBand0Len == sizeMetaBand);

    std::vector<bool> metaBand1Packet = bitstream[1];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaBand1Packet.begin() + beginIdx,
                                   metaBand1Packet.begin() + beginIdx +
                                       haptics::io::H_PAYLOAD_LENGTH);
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
    CHECK(metaBand1Len == sizeMetaBand);

    std::vector<bool> metaBand2Packet = bitstream[2];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaBand2Packet.begin() + beginIdx,
                                   metaBand2Packet.begin() + beginIdx +
                                       haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaBand2Len =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
    CHECK(metaBand2Len == sizeMetaBand);
  }

  SECTION("Read metadataBand") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);
    std::vector<bool> metaHaptic = bitstream[0];
    std::vector<bool> metaPerception0 = bitstream[1];
    std::vector<bool> metaPerception1 = bitstream[2];
    std::vector<bool> metaTrack0 = bitstream[3];
    std::vector<bool> metaTrack1 = bitstream[4];
    std::vector<bool> metaTrack2 = bitstream[5];
    haptics::types::Haptics readHaptic;
    IOStream::Buffer buffer = IOStream::initializeBuffer(readHaptic);
    succeed = IOStream::readNALu(readHaptic, metaHaptic, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaTrack0, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaTrack1, buffer);
    succeed &= IOStream::readNALu(readHaptic, metaTrack2, buffer);

    REQUIRE(succeed);
    // Check Haptic Band information
  }
};
// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write haptic experience as streamable packet") {
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

  const auto testingBandType_band2 = haptics::types::BandType::VectorialWave;
  const auto testingCurveType_band2 = haptics::types::CurveType::Unknown;
  const int testingWindowLength_band2 = 0;
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

  SECTION("write file") {
    std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
    bool succeed = IOStream::writePacket(testingHaptic, bitstream);

    REQUIRE(succeed);

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

    const uintmax_t sizeMetaRefDev1 = 304;
    const uintmax_t sizeMetaRefDev2 = 552;
    const uintmax_t sizeMetaRefDev3 = 61;
    const uintmax_t sizeMetaPerception0 = 1401;
    const uintmax_t sizeMetaPerception1 = 452;
    const uintmax_t sizeMetaPerception = sizeMetaPerception0 + sizeMetaPerception1;
    const uintmax_t sizeMetaTrack0 = 569;
    const uintmax_t sizeMetaTrack1 = 401;
    const uintmax_t sizeMetaTrack2 = 441;
    const uintmax_t sizeMetaTrack = sizeMetaTrack0 + sizeMetaTrack1 + sizeMetaTrack2;
    const uintmax_t sizeMetaBand = 375;

    // CHECK metadata experience length is correct
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

    CHECK(metaExpLen == sizeMetaExperience);

    // CHECK metadata Perception length is correct
    std::vector<bool> metaPercPacket = bitstream[1];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaPercPacket.begin() + beginIdx,
                          metaPercPacket.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaPercLen =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());

    CHECK(metaPercLen == sizeMetaPerception);

    // CHECK metadata Track length is correct
    std::vector<bool> metaTrackPacket = bitstream[2];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaTrackPacket.begin() + beginIdx,
                          metaTrackPacket.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaTrackLen =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());

    CHECK(metaTrackLen == sizeMetaTrack);

    // CHECK metadata Band length is correct
    std::vector<bool> metaBandPacket = bitstream[3];
    beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
    packetLength =
        std::vector<bool>(metaBandPacket.begin() + beginIdx,
                          metaBandPacket.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
    packetLengthStr.clear();
    for (auto c : packetLength) {
      if (c) {
        packetLengthStr += "1";
      } else {
        packetLengthStr += "0";
      }
    }
    int metaBandLen =
        static_cast<int>(std::bitset<haptics::io::H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());

    CHECK(metaBandLen == sizeMetaBand);
  }

  // SECTION("read file") {
  //   const uintmax_t startedFileSize = std::filesystem::file_size(filename);

  //  haptics::types::Haptics res;
  //  bool succeed = IOBinary::loadFile(filename, res);

  //  REQUIRE(succeed);
  //  CHECK(std::filesystem::file_size(filename) == startedFileSize);
  //  CHECK(res.getVersion() == testingVersion);
  //  CHECK(res.getDate() == testingDate);
  //  CHECK(res.getDescription() == testingDescription);

  //  // CHECK avatars
  //  REQUIRE(res.getAvatarsSize() == 2);
  //  CHECK(res.getAvatarAt(0).getId() == testingId_avatar1);
  //  CHECK(res.getAvatarAt(0).getLod() == testingLod_avatar1);
  //  CHECK(res.getAvatarAt(0).getType() == testingType_avatar1);
  //  CHECK(res.getAvatarAt(0).getMesh() == testingMesh_avatar1);
  //  CHECK(res.getAvatarAt(1).getId() == testingId_avatar2);
  //  CHECK(res.getAvatarAt(1).getLod() == testingLod_avatar2);
  //  CHECK(res.getAvatarAt(1).getType() == testingType_avatar2);

  //  REQUIRE(res.getPerceptionsSize() == 2);

  //  // CHECK perception 0
  //  CHECK(res.getPerceptionAt(0).getAvatarId() == testingAvatarId_perception0);
  //  CHECK(res.getPerceptionAt(0).getDescription() == testingDescription_perception0);
  //  CHECK(res.getPerceptionAt(0).getId() == testingId_perception0);
  //  CHECK(res.getPerceptionAt(0).getPerceptionModality() ==
  //  testingPerceptionModality_perception0); REQUIRE(res.getPerceptionAt(0).getTracksSize() == 2);

  //  // CHECK track 0
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBodyPartMask() == testingBodyPartMask_track0);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getDescription() == testingDescription_track0);
  //  CHECK(std::fabs(res.getPerceptionAt(0).getTrackAt(0).getGain() - testingGain_track0) <
  //        floatPrecision);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getId() == testingId_track0);
  //  CHECK(std::fabs(res.getPerceptionAt(0).getTrackAt(0).getMixingWeight() -
  //                  testingMixingWeight_track0) < floatPrecision);
  //  REQUIRE(res.getPerceptionAt(0).getTrackAt(0).getVerticesSize() ==
  //          testingVertices_track0.size());
  //  for (int i = 0; i < static_cast<int>(testingVertices_track0.size()); i++) {
  //    CHECK(res.getPerceptionAt(0).getTrackAt(0).getVertexAt(i) == testingVertices_track0.at(i));
  //  }

  //  // CHECK band
  //  REQUIRE(res.getPerceptionAt(0).getTrackAt(0).getBandsSize() == 1);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getBandType() ==
  //  testingBandType_band0); CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getCurveType()
  //  ==
  //        testingCurveType_band0);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getLowerFrequencyLimit() ==
  //        testingLowerFrequencyLimit_band0);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getUpperFrequencyLimit() ==
  //        testingUpperFrequencyLimit_band0);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getWindowLength() ==
  //        testingWindowLength_band0);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getEffectsSize() == 1);

  //  // CHECK effect
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getEffectAt(0).getPosition() ==
  //        testingPosition_effect0);
  //  CHECK(std::fabs(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getEffectAt(0).getPhase() -
  //                  testingPhase_effect0) < floatPrecision);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getEffectAt(0).getBaseSignal() ==
  //        testingBaseSignal_effect0);
  //  REQUIRE(res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getEffectAt(0).getKeyframesSize() ==
  //          testingKeyframes_effect0.size());
  //  for (int i = 0; i < static_cast<int>(testingKeyframes_effect0.size()); i++) {
  //    haptics::types::Keyframe resKeyframe =
  //        res.getPerceptionAt(0).getTrackAt(0).getBandAt(0).getEffectAt(0).getKeyframeAt(i);
  //    REQUIRE(resKeyframe.getRelativePosition().has_value());
  //    REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  //    CHECK_FALSE(resKeyframe.getFrequencyModulation().has_value());
  //    CHECK(resKeyframe.getRelativePosition().value() ==
  //          std::get<0>(testingKeyframes_effect0.at(i)));
  //    CHECK(std::fabs(resKeyframe.getAmplitudeModulation().value() -
  //                    std::get<1>(testingKeyframes_effect0.at(i))) < floatPrecision);
  //  }

  //  // CHECK track 1
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getBodyPartMask() == testingBodyPartMask_track1);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getDescription() == testingDescription_track1);
  //  CHECK(std::fabs(res.getPerceptionAt(0).getTrackAt(1).getGain() - testingGain_track1) <
  //        floatPrecision);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getId() == testingId_track1);
  //  CHECK(std::fabs(res.getPerceptionAt(0).getTrackAt(1).getMixingWeight() -
  //                  testingMixingWeight_track1) < floatPrecision);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getVerticesSize() == 0);

  //  // CHECK band
  //  REQUIRE(res.getPerceptionAt(0).getTrackAt(1).getBandsSize() == 1);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getBandType() ==
  //  testingBandType_band1); CHECK(res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getCurveType()
  //  ==
  //        testingCurveType_band1);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getLowerFrequencyLimit() ==
  //        testingLowerFrequencyLimit_band1);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getUpperFrequencyLimit() ==
  //        testingUpperFrequencyLimit_band1);
  //  CHECK(res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getWindowLength() ==
  //        testingWindowLength_band1);
  //  REQUIRE(res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getEffectsSize() ==
  //          testingKeyframes_effect1.size());

  //  // CHECK effects
  //  for (int i = 0; i < static_cast<int>(testingKeyframes_effect1.size()); i++) {
  //    haptics::types::Effect resEffect =
  //        res.getPerceptionAt(0).getTrackAt(1).getBandAt(0).getEffectAt(i);
  //    CHECK(resEffect.getPosition() == std::get<0>(testingKeyframes_effect1.at(i)));
  //    CHECK(std::fabs(resEffect.getPhase() - 0) < floatPrecision);
  //    REQUIRE(resEffect.getKeyframesSize() == 1);
  //    REQUIRE(resEffect.getKeyframeAt(0).getRelativePosition().has_value());
  //    REQUIRE(resEffect.getKeyframeAt(0).getAmplitudeModulation().has_value());
  //    REQUIRE(resEffect.getKeyframeAt(0).getFrequencyModulation().has_value());
  //    CHECK(resEffect.getKeyframeAt(0).getRelativePosition().value() == 0);
  //    CHECK(std::fabs(resEffect.getKeyframeAt(0).getAmplitudeModulation().value() -
  //                    std::get<1>(testingKeyframes_effect1.at(i))) < floatPrecision);
  //    CHECK(resEffect.getKeyframeAt(0).getFrequencyModulation().value() ==
  //          std::get<2>(testingKeyframes_effect1.at(i)));
  //  }

  //  // CHECK perception 1
  //  CHECK(res.getPerceptionAt(1).getAvatarId() == testingAvatarId_perception1);
  //  CHECK(res.getPerceptionAt(1).getDescription() == testingDescription_perception1);
  //  CHECK(res.getPerceptionAt(1).getId() == testingId_perception1);
  //  CHECK(res.getPerceptionAt(1).getPerceptionModality() ==
  //  testingPerceptionModality_perception1); CHECK(res.getPerceptionAt(1).getReferenceDevicesSize()
  //  == 0); REQUIRE(res.getPerceptionAt(1).getTracksSize() == 1);

  //  // CHECK track
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBodyPartMask() == testingBodyPartMask_track2);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getDescription() == testingDescription_track2);
  //  CHECK(std::fabs(res.getPerceptionAt(1).getTrackAt(0).getGain() - testingGain_track2) <
  //        floatPrecision);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getId() == testingId_track2);
  //  CHECK(std::fabs(res.getPerceptionAt(1).getTrackAt(0).getMixingWeight() -
  //                  testingMixingWeight_track2) < floatPrecision);
  //  REQUIRE(res.getPerceptionAt(1).getTrackAt(0).getVerticesSize() ==
  //          testingVertices_track2.size());
  //  for (int i = 0; i < static_cast<int>(testingVertices_track2.size()); i++) {
  //    CHECK(res.getPerceptionAt(1).getTrackAt(0).getVertexAt(i) == testingVertices_track2.at(i));
  //  }

  //  // CHECK band
  //  REQUIRE(res.getPerceptionAt(1).getTrackAt(0).getBandsSize() == 1);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getBandType() ==
  //  testingBandType_band2); CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getCurveType()
  //  ==
  //        testingCurveType_band2);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getLowerFrequencyLimit() ==
  //        testingLowerFrequencyLimit_band2);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getUpperFrequencyLimit() ==
  //        testingUpperFrequencyLimit_band2);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getWindowLength() ==
  //        testingWindowLength_band2);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getEffectsSize() == 1);
  //  CHECK(res.getPerceptionAt(1).getTrackAt(0).getBandAt(0).getEffectAt(0).getKeyframesSize() ==
  //  0);

  //  std::filesystem::remove(filename);
  //  CHECK(!std::filesystem::is_regular_file(filename));
  //}
}