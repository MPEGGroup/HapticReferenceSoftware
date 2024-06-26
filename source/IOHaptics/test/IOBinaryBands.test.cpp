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

#include <IOHaptics/include/IOBinaryBands.h>
#include <IOHaptics/include/IOBinaryFields.h>
#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using haptics::io::IOBinaryBands;
using haptics::io::IOBinaryPrimitives;

const std::string filename = "testing_IOBinaryBands.bin";
constexpr float floatPrecision = 0.01;
constexpr unsigned int timescale = 1000;

auto addEffect(
    haptics::types::Band &myBand, const int position, const float phase,
    const haptics::types::BaseSignal baseSignal, const haptics::types::EffectType effectType,
    const std::vector<std::tuple<int, std::optional<float>, std::optional<int>>> &keyframes)
    -> void {
  haptics::types::Effect myEffect(position, phase, baseSignal, effectType);
  for (auto keyframeValue : keyframes) {
    if (std::get<1>(keyframeValue).has_value()) {
      myEffect.addAmplitudeAt(std::get<1>(keyframeValue).value(), std::get<0>(keyframeValue));
    }
    if (std::get<2>(keyframeValue).has_value()) {
      myEffect.addFrequencyAt(std::get<2>(keyframeValue).value(), std::get<0>(keyframeValue));
    }
  }

  myBand.addEffect(myEffect);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandHeader on curve") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::Curve;
  const haptics::types::CurveType testingCurveType = haptics::types::CurveType::Cubic;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 300;
  haptics::types::Band testingBand(testingBandType, testingCurveType, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);
  haptics::types::Effect testingEffect(0, 0, haptics::types::BaseSignal::Sine,
                                       haptics::types::EffectType::Basis);
  const int expectedKeyframeCount = 42;
  for (int i = 0; i < expectedKeyframeCount; i++) {
    const float amplitude = static_cast<float>(i) / expectedKeyframeCount;
    testingEffect.addAmplitudeAt(amplitude, i);
  }
  testingBand.addEffect(testingEffect);

  SECTION("write band header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);
    std::vector<bool> output;
    IOBinaryBands::writeBandHeader(testingBand, output, timescale);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();
    auto bitStreamSize = haptics::io::MDBAND_BAND_TYPE + haptics::io::MDBAND_CURVE_TYPE +
                         haptics::io::MDBAND_LOW_FREQ + haptics::io::MDBAND_UP_FREQ +
                         haptics::io::MDBAND_EFFECT_COUNT;
    auto byteStreamSize = bitStreamSize % haptics::io::BYTE_SIZE == 0
                              ? bitStreamSize / haptics::io::BYTE_SIZE
                              : (bitStreamSize / haptics::io::BYTE_SIZE) + 1;
    CHECK(std::filesystem::file_size(filename) == static_cast<uintmax_t>(byteStreamSize));
  }

  SECTION("read band header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandHeader(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getBandType() == testingBandType);
    CHECK(res.getCurveType() == testingCurveType);
    CHECK(res.getLowerFrequencyLimit() == testingLowerFrequencyLimit);
    CHECK(res.getUpperFrequencyLimit() == testingUpperFrequencyLimit);
    CHECK(res.getEffectsSize() == 1);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandHeader on transient") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::Transient;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 300;
  haptics::types::Band testingBand(testingBandType, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);
  const int expectedTransientCount = 42;
  for (int i = 0; i < expectedTransientCount; i++) {
    haptics::types::Effect testingEffect;
    haptics::types::Keyframe testingKeyframe;
    testingEffect.addKeyframe(testingKeyframe);
    testingBand.addEffect(testingEffect);
  }

  SECTION("write band header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandHeader(testingBand, output, timescale);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    CHECK(std::filesystem::file_size(filename) == 7);
  }

  SECTION("read band header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandHeader(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getBandType() == testingBandType);
    CHECK(res.getLowerFrequencyLimit() == testingLowerFrequencyLimit);
    CHECK(res.getUpperFrequencyLimit() == testingUpperFrequencyLimit);
    CHECK(res.getEffectsSize() == expectedTransientCount);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandHeader on vectorial wave") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::VectorialWave;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 300;
  haptics::types::Band testingBand(testingBandType, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);
  const int expectedEffectCount = 42;
  for (int i = 0; i < expectedEffectCount; i++) {
    haptics::types::Effect testingEffect;
    testingBand.addEffect(testingEffect);
  }

  SECTION("write band header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandHeader(testingBand, output, timescale);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    CHECK(std::filesystem::file_size(filename) == 7);
  }

  SECTION("read band header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandHeader(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getBandType() == testingBandType);
    CHECK(res.getLowerFrequencyLimit() == testingLowerFrequencyLimit);
    CHECK(res.getUpperFrequencyLimit() == testingUpperFrequencyLimit);
    CHECK(res.getEffectsSize() == expectedEffectCount);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandHeader on wavelet wave") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::WaveletWave;
  const int testingBlockLength = 128;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 8000;
  haptics::types::Band testingBand(testingBandType, testingBlockLength, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);
  const int expectedEffectCount = 42;
  for (int i = 0; i < expectedEffectCount; i++) {
    haptics::types::Effect testingEffect;
    testingBand.addEffect(testingEffect);
  }

  SECTION("write band header") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandHeader(testingBand, output, timescale);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    CHECK(std::filesystem::file_size(filename) == 8);
  }

  SECTION("read band header") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res;
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandHeader(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    CHECK(res.getBandType() == testingBandType);
    CHECK(res.getBlockLength() == testingBlockLength);
    CHECK(res.getLowerFrequencyLimit() == testingLowerFrequencyLimit);
    CHECK(res.getUpperFrequencyLimit() == testingUpperFrequencyLimit);
    CHECK(res.getEffectsSize() == expectedEffectCount);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandBody on curve") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::Curve;
  const haptics::types::CurveType testingCurveType = haptics::types::CurveType::Cubic;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 300;
  haptics::types::Band testingBand(testingBandType, testingCurveType, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);
  const int expectedKeyframeCount = 42;
  haptics::types::Effect testingEffect;
  for (int i = 0; i < expectedKeyframeCount; i++) {
    const auto amplitude = static_cast<float>(i) / expectedKeyframeCount;
    testingEffect.addAmplitudeAt(amplitude, i);
  }
  testingBand.addEffect(testingEffect);

  SECTION("write band body") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandBody(testingBand, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    auto bitStreamSize =
        haptics::io::EFFECT_TYPE + haptics::io::EFFECT_POSITION +
        haptics::io::EFFECT_KEYFRAME_COUNT +
        expectedKeyframeCount * (haptics::io::KEYFRAME_AMPLITUDE + haptics::io::KEYFRAME_POSITION);
    auto byteStreamSize = bitStreamSize % haptics::io::BYTE_SIZE == 0
                              ? bitStreamSize / haptics::io::BYTE_SIZE
                              : (bitStreamSize / haptics::io::BYTE_SIZE) + 1;
    CHECK(std::filesystem::file_size(filename) == static_cast<uintmax_t>(byteStreamSize));
  }

  SECTION("read band body") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res(testingBandType, testingCurveType, testingLowerFrequencyLimit,
                             testingUpperFrequencyLimit);
    haptics::types::Effect resEffect;
    res.addEffect(resEffect);
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandBody(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getEffectsSize() == 1);
    REQUIRE(res.getEffectAt(0).getKeyframesSize() == expectedKeyframeCount);
    for (int i = 0; i < expectedKeyframeCount; i++) {
      haptics::types::Keyframe keyframe = res.getEffectAt(0).getKeyframeAt(i);
      CHECK(keyframe.getRelativePosition().has_value());
      CHECK(keyframe.getRelativePosition().value() == i);
      CHECK(keyframe.getAmplitudeModulation().has_value());
      CHECK(std::fabs(keyframe.getAmplitudeModulation().value() -
                      (static_cast<float>(i) / expectedKeyframeCount)) < floatPrecision);
      CHECK_FALSE(keyframe.getFrequencyModulation().has_value());
    }
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandBody on transient") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::Transient;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 300;
  haptics::types::Band testingBand(testingBandType, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);
  const int expectedTransientCount = 12;
  for (int i = 0; i < expectedTransientCount; i++) {
    haptics::types::Effect testingEffect(i, 0, haptics::types::BaseSignal::Sine,
                                         haptics::types::EffectType::Basis);
    const auto amplitude = static_cast<float>(i) / expectedTransientCount;
    const int frequency = 90;
    haptics::types::Keyframe testingKeyframe(0, amplitude, frequency);
    testingEffect.addKeyframe(testingKeyframe);
    testingBand.addEffect(testingEffect);
  }

  SECTION("write band body") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandBody(testingBand, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    auto bitStreamSize = expectedTransientCount *
                         (haptics::io::EFFECT_TYPE + haptics::io::EFFECT_POSITION +
                          haptics::io::EFFECT_KEYFRAME_COUNT + haptics::io::KEYFRAME_AMPLITUDE +
                          haptics::io::KEYFRAME_POSITION + haptics::io::KEYFRAME_FREQUENCY);
    auto byteStreamSize = bitStreamSize % haptics::io::BYTE_SIZE == 0
                              ? bitStreamSize / haptics::io::BYTE_SIZE
                              : (bitStreamSize / haptics::io::BYTE_SIZE) + 1;
    CHECK(std::filesystem::file_size(filename) == static_cast<uintmax_t>(byteStreamSize));
  }

  SECTION("read band body") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res(testingBandType, testingLowerFrequencyLimit,
                             testingUpperFrequencyLimit);
    for (int i = 0; i < expectedTransientCount; i++) {
      haptics::types::Effect resEffect(0, 0, haptics::types::BaseSignal::Square,
                                       haptics::types::EffectType::Basis);
      res.addEffect(resEffect);
    }
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandBody(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getEffectsSize() == expectedTransientCount);
    for (int i = 0; i < expectedTransientCount; i++) {
      haptics::types::Effect effect = res.getEffectAt(i);
      REQUIRE(effect.getKeyframesSize() == 1);
      haptics::types::Keyframe keyframe = res.getEffectAt(i).getKeyframeAt(0);
      CHECK(effect.getPosition() == i);
      CHECK(keyframe.getAmplitudeModulation().has_value());
      CHECK(std::fabs(keyframe.getAmplitudeModulation().value() -
                      (static_cast<float>(i) / expectedTransientCount)) < floatPrecision);
      CHECK(keyframe.getFrequencyModulation().has_value());
      CHECK(keyframe.getFrequencyModulation().value() == 90);
    }
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandBody on vectorial wave") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::VectorialWave;
  const int testingLowerFrequencyLimit = 65;
  const int testingUpperFrequencyLimit = 300;
  haptics::types::Band testingBand(testingBandType, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);

  const int testingEffect1_position = 42;
  const float testingEffect1_phase = 0;
  const haptics::types::BaseSignal testingEffect1_baseSignal =
      haptics::types::BaseSignal::SawToothUp;
  const std::vector<std::tuple<int, std::optional<float>, std::optional<int>>>
      testingEffect1_keyframes = {{0, .1F, 90}, {36, .1F, 90}};
  addEffect(testingBand, testingEffect1_position, testingEffect1_phase, testingEffect1_baseSignal,
            haptics::types::EffectType::Basis, testingEffect1_keyframes);

  const int testingEffect2_position = 654;
  const float testingEffect2_phase = 2.65;
  const haptics::types::BaseSignal testingEffect2_baseSignal = haptics::types::BaseSignal::Triangle;
  const std::vector<std::tuple<int, std::optional<float>, std::optional<int>>>
      testingEffect2_keyframes = {
          {0, .1F, 90}, {72, .954F, 90}, {1000, std::nullopt, 300}, {1036, 0.0F, std::nullopt}};
  addEffect(testingBand, testingEffect2_position, testingEffect2_phase, testingEffect2_baseSignal,
            haptics::types::EffectType::Basis, testingEffect2_keyframes);

  SECTION("write band body") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandBody(testingBand, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    auto bitStreamSize =
        2 * (haptics::io::EFFECT_TYPE + haptics::io::EFFECT_POSITION + haptics::io::EFFECT_PHASE +
             haptics::io::EFFECT_BASE_SIGNAL + haptics::io::EFFECT_KEYFRAME_COUNT) +
        (2 + 4) * (haptics::io::KEYFRAME_MASK + haptics::io::KEYFRAME_POSITION) +
        (2 + 3) * haptics::io::KEYFRAME_AMPLITUDE + (2 + 3) * haptics::io::KEYFRAME_FREQUENCY;
    auto byteStreamSize = bitStreamSize % haptics::io::BYTE_SIZE == 0
                              ? bitStreamSize / haptics::io::BYTE_SIZE
                              : (bitStreamSize / haptics::io::BYTE_SIZE) + 1;
    CHECK(std::filesystem::file_size(filename) == static_cast<uintmax_t>(byteStreamSize));
  }

  SECTION("read band body") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res(testingBandType, testingLowerFrequencyLimit,
                             testingUpperFrequencyLimit);
    haptics::types::Effect resEffect1;
    haptics::types::Effect resEffect2;
    res.addEffect(resEffect1);
    res.addEffect(resEffect2);
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandBody(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getEffectsSize() == 2);

    resEffect1 = res.getEffectAt(0);
    CHECK(resEffect1.getPosition() == testingEffect1_position);
    CHECK(std::fabs(resEffect1.getPhaseOrDefault() - (testingEffect1_phase)) < floatPrecision);
    CHECK(resEffect1.getBaseSignalOrDefault() == testingEffect1_baseSignal);
    REQUIRE(resEffect1.getKeyframesSize() == testingEffect1_keyframes.size());
    for (int i = 0; i < static_cast<int>(resEffect1.getKeyframesSize()); i++) {
      haptics::types::Keyframe resKeyframe = resEffect1.getKeyframeAt(i);
      std::tuple<int, std::optional<float>, std::optional<int>> expectedKeyframeValue =
          testingEffect1_keyframes.at(i);

      CHECK(resKeyframe.getRelativePosition().has_value());
      CHECK(resKeyframe.getRelativePosition().value() == std::get<0>(expectedKeyframeValue));
      CHECK(resKeyframe.getAmplitudeModulation().has_value() ==
            std::get<1>(expectedKeyframeValue).has_value());
      if (resKeyframe.getAmplitudeModulation().has_value()) {
        CHECK(std::fabs(resKeyframe.getAmplitudeModulation().value() -
                        std::get<1>(expectedKeyframeValue).value()) < floatPrecision);
      }
      CHECK(resKeyframe.getFrequencyModulation().has_value() ==
            std::get<2>(expectedKeyframeValue).has_value());
      if (resKeyframe.getFrequencyModulation().has_value()) {
        CHECK(std::fabs(resKeyframe.getFrequencyModulation().value() -
                        std::get<2>(expectedKeyframeValue).value()) < floatPrecision);
      }
    }

    resEffect2 = res.getEffectAt(1);
    CHECK(resEffect2.getPosition() == testingEffect2_position);
    CHECK(std::fabs(resEffect2.getPhaseOrDefault() - testingEffect2_phase) < floatPrecision);
    CHECK(resEffect2.getBaseSignalOrDefault() == testingEffect2_baseSignal);
    REQUIRE(resEffect2.getKeyframesSize() == testingEffect2_keyframes.size());
    for (int i = 0; i < static_cast<int>(resEffect2.getKeyframesSize()); i++) {
      haptics::types::Keyframe resKeyframe = resEffect2.getKeyframeAt(i);
      std::tuple<int, std::optional<float>, std::optional<int>> expectedKeyframeValue =
          testingEffect2_keyframes.at(i);

      CHECK(resKeyframe.getRelativePosition().has_value());
      CHECK(resKeyframe.getRelativePosition().value() == std::get<0>(expectedKeyframeValue));
      CHECK(resKeyframe.getAmplitudeModulation().has_value() ==
            std::get<1>(expectedKeyframeValue).has_value());
      if (resKeyframe.getAmplitudeModulation().has_value()) {
        CHECK(std::fabs(resKeyframe.getAmplitudeModulation().value() -
                        std::get<1>(expectedKeyframeValue).value()) < floatPrecision);
      }
      CHECK(resKeyframe.getFrequencyModulation().has_value() ==
            std::get<2>(expectedKeyframeValue).has_value());
      if (resKeyframe.getFrequencyModulation().has_value()) {
        CHECK(std::fabs(resKeyframe.getFrequencyModulation().value() -
                        std::get<2>(expectedKeyframeValue).value()) < floatPrecision);
      }
    }
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("write/read BandBody on wavelet") {
  const haptics::types::BandType testingBandType = haptics::types::BandType::WaveletWave;
  const int testingBlockLength = 255;
  const int testingLowerFrequencyLimit = 75;
  const int testingUpperFrequencyLimit = 1000;
  haptics::types::Band testingBand(testingBandType, testingBlockLength, testingLowerFrequencyLimit,
                                   testingUpperFrequencyLimit);

  SECTION("write band body") {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    REQUIRE(file);

    std::vector<bool> output;
    IOBinaryBands::writeBandBody(testingBand, output);
    IOBinaryPrimitives::fillBitset(output);
    IOBinaryPrimitives::writeBitset(output, file);
    file.close();

    CHECK(std::filesystem::file_size(filename) == 0);
  }

  SECTION("read band body") {
    const uintmax_t startedFileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    REQUIRE(file);

    haptics::types::Band res(testingBandType, testingBlockLength, testingLowerFrequencyLimit,
                             testingUpperFrequencyLimit);
    std::vector<bool> unusedBits;
    bool succeed = IOBinaryBands::readBandBody(res, file, unusedBits, timescale);
    file.close();

    REQUIRE(succeed);
    CHECK(std::filesystem::file_size(filename) == startedFileSize);
    REQUIRE(res.getEffectsSize() == 0);
  }
}
