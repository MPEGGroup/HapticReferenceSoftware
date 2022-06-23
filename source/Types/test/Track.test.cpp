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

#include <Types/include/Band.h>
#include <Types/include/Effect.h>
#include <Types/include/Track.h>
#include <catch2/catch.hpp>

const int id = -1;
const std::string desctription = "placeholder";
const float gain = 1;
const float mixingWeight = .5;
const int bodypartMask = 42;
const int lowF = 0;
const int highF = 1000;
const haptics::types::Direction direction(127, 1, -34);

TEST_CASE("Track::generateBand without parameters", "[generateBand][withoutParameters]") {
  const int bandcount = 19;

  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);
  for (int i = 0; i < bandcount; i++) {
    haptics::types::Band b(haptics::types::BandType::Wave, haptics::types::CurveType::Unknown,
                           haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
    t.addBand(b);
  }

  REQUIRE(t.getBandsSize() == static_cast<size_t>(bandcount));

  haptics::types::Band *res = t.generateBand();

  CHECK(t.getBandsSize() == static_cast<size_t>(bandcount) + 1);
  CHECK_FALSE(res == nullptr);
}

TEST_CASE("Track::generateBand with parameters", "[generateBand][withParameters]") {
  const int bandcount = 19;
  const haptics::types::BandType testingBandType = haptics::types::BandType::Curve;
  const haptics::types::CurveType testingCurveType = haptics::types::CurveType::Linear;
  const haptics::types::EncodingModality testingEncodingModality =
      haptics::types::EncodingModality::Wavelet;
  const int testingWindowLength = 32;
  const int testingMinF = 65;
  const int testingMaxF = 300;

  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);
  for (int i = 0; i < bandcount; i++) {
    haptics::types::Band b(haptics::types::BandType::Wave, haptics::types::CurveType::Unknown,
                           haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
    t.addBand(b);
  }

  REQUIRE(t.getBandsSize() == static_cast<size_t>(bandcount));

  haptics::types::Band *res =
      t.generateBand(testingBandType, testingCurveType, testingEncodingModality,
                     testingWindowLength, testingMinF, testingMaxF);

  CHECK(t.getBandsSize() == static_cast<size_t>(bandcount) + 1);
  CHECK_FALSE(res == nullptr);
  CHECK(res->getBandType() == testingBandType);
  CHECK(res->getCurveType() == testingCurveType);
  CHECK(res->getEncodingModality() == testingEncodingModality);
  CHECK(res->getWindowLength() == testingWindowLength);
  CHECK(res->getLowerFrequencyLimit() == testingMinF);
  CHECK(res->getUpperFrequencyLimit() == testingMaxF);
  CHECK(res->getEffectsSize() == 0);
}

TEST_CASE("Track::findWaveBandAvailable without band", "[findWaveBandAvailable][empty]") {
  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);

  const int testingStart = 42;
  const int testingLength = 24;
  haptics::types::Band *res =
      t.findBandAvailable(testingStart, testingLength, haptics::types::BandType::Wave,
                          haptics::types::EncodingModality::Vectorial);

  CHECK(res == nullptr);
}

TEST_CASE("Track::findWaveBandAvailable with overlapping effects",
          "[findWaveBandAvailable][overlappingEffect]") {
  const int effectPos = 24;
  const int effectSize = 42;
  const int effectF = 75;
  const float effectA = 0.75;

  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);
  haptics::types::Band b(haptics::types::BandType::Wave, haptics::types::CurveType::Unknown,
                         haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
  haptics::types::Effect e(effectPos, 0.0, haptics::types::BaseSignal::Triangle);
  haptics::types::Keyframe kf1(0, effectA, effectF);
  haptics::types::Keyframe kf2(effectSize, effectA, effectF);
  e.addKeyframe(kf1);
  e.addKeyframe(kf2);
  b.addEffect(e);
  t.addBand(b);

  const int testingStart = 42;
  const int testingLength = 24;
  haptics::types::Band *res =
      t.findBandAvailable(testingStart, testingLength, haptics::types::BandType::Wave,
                          haptics::types::EncodingModality::Vectorial);

  CHECK(res == nullptr);
}

TEST_CASE("Track::findWaveBandAvailable band available but with types",
          "[findWaveBandAvailable][withWrongType]") {
  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);
  haptics::types::Band b1(haptics::types::BandType::Curve, haptics::types::CurveType::Cubic,
                          haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
  t.addBand(b1);
  haptics::types::Band b2(haptics::types::BandType::Transient, haptics::types::CurveType::Linear,
                          haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
  t.addBand(b2);

  const int testingStart = 42;
  const int testingLength = 24;
  haptics::types::Band *res =
      t.findBandAvailable(testingStart, testingLength, haptics::types::BandType::Wave,
                          haptics::types::EncodingModality::Vectorial);

  CHECK(res == nullptr);
}

TEST_CASE("Track::findWaveBandAvailable empty band available", "[findWaveBandAvailable][empty]") {
  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);
  haptics::types::Band b(haptics::types::BandType::Wave, haptics::types::CurveType::Linear,
                         haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
  t.addBand(b);

  const int testingStart = 42;
  const int testingLength = 24;
  haptics::types::Band *res =
      t.findBandAvailable(testingStart, testingLength, haptics::types::BandType::Wave,
                          haptics::types::EncodingModality::Vectorial);

  REQUIRE(res != nullptr);
  CHECK(res == &t.getBandAt(0));
  CHECK(res->getBandType() == haptics::types::BandType::Wave);
  CHECK(res->getCurveType() == haptics::types::CurveType::Linear);
  CHECK(res->getEncodingModality() == haptics::types::EncodingModality::Vectorial);
  CHECK(res->getWindowLength() == 0);
  CHECK(res->getLowerFrequencyLimit() == lowF);
  CHECK(res->getUpperFrequencyLimit() == highF);
  CHECK(res->getEffectsSize() == 0);
}

TEST_CASE("Track::findWaveBandAvailable with correct return",
          "[findWaveBandAvailable][overlappingEffect]") {
  const int effect1Size = 1000;
  const int effect1F = 936;
  const int effect2Pos = 24;
  const int effect2Size = 42;
  const int effect2F = 75;
  const int effect3Pos = 150;
  const int effect3Size = 42;
  const int effect3F = 10;

  haptics::types::Track t(id, desctription, gain, mixingWeight, bodypartMask, direction);
  haptics::types::Band b1(haptics::types::BandType::Wave, haptics::types::CurveType::Cubic,
                          haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
  haptics::types::Effect e1(0, 0.0, haptics::types::BaseSignal::Triangle);
  haptics::types::Keyframe kf1(0, 1.0, 0);
  haptics::types::Keyframe kf2(effect1Size, 1.0, effect1F);
  e1.addKeyframe(kf1);
  e1.addKeyframe(kf2);
  b1.addEffect(e1);
  t.addBand(b1);
  haptics::types::Band b2(haptics::types::BandType::Wave, haptics::types::CurveType::Linear,
                          haptics::types::EncodingModality::Vectorial, 0, lowF, highF);
  haptics::types::Effect e2(effect2Pos, 0.0, haptics::types::BaseSignal::Triangle);
  haptics::types::Keyframe kf3(0, 1.0, effect2F);
  haptics::types::Keyframe kf4(effect2Size, 1.0, effect2F);
  e2.addKeyframe(kf3);
  e2.addKeyframe(kf4);
  b2.addEffect(e2);
  haptics::types::Effect e3(effect3Pos, 0.0, haptics::types::BaseSignal::Triangle);
  haptics::types::Keyframe kf5(0, 1.0, effect3F);
  haptics::types::Keyframe kf6(effect3Size, 1.0, effect3F);
  e3.addKeyframe(kf5);
  e3.addKeyframe(kf6);
  b2.addEffect(e3);
  t.addBand(b2);

  const int testingPosition = 100;
  const int testingDuration = 24;
  haptics::types::Band *res =
      t.findBandAvailable(testingPosition, testingDuration, haptics::types::BandType::Wave,
                          haptics::types::EncodingModality::Vectorial);

  REQUIRE(res != nullptr);
  CHECK(res == &t.getBandAt(1));
  haptics::types::Effect e;
  for (uint32_t i = 0; i < res->getEffectsSize(); i++) {
    e = res->getEffectAt((int)i);
    REQUIRE_FALSE(res->isOverlapping(e, testingPosition, testingPosition + testingDuration));
  }
}
