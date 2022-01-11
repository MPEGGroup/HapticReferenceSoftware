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

#include <catch2/catch.hpp>
#include <Types/include/Keyframe.h>
#include <Types/include/Effect.h>

using haptics::types::BaseSignal;
using haptics::types::Keyframe;
using haptics::types::Effect;

TEST_CASE("addKeyframe with only position", "[addKeyframe]") {
  Effect e(0, 1, BaseSignal::SawToothUp);
  const size_t n = e.getKeyframesSize();

  const int testingPosition = 42;
  e.addKeyframe(testingPosition, std::nullopt, std::nullopt);

  REQUIRE(e.getKeyframesSize() == n + 1);
  Keyframe testedKeyframe = e.getKeyframeAt(static_cast<int>(e.getKeyframesSize()) - 1);
  CHECK(testedKeyframe.getRelativePosition() == testingPosition);
  CHECK_FALSE(testedKeyframe.getAmplitudeModulation().has_value());
  CHECK_FALSE(testedKeyframe.getFrequencyModulation().has_value());
}

TEST_CASE("addKeyframe with amplitude", "[addKeyframe]") {
  Effect e(0, 1, BaseSignal::SawToothUp);
  const size_t n = e.getKeyframesSize();

  const int testingPosition = 42;
  const double testingAmplitude = .7654;
  e.addKeyframe(testingPosition, testingAmplitude, std::nullopt);

  REQUIRE(e.getKeyframesSize() == n + 1);
  Keyframe testedKeyframe = e.getKeyframeAt(static_cast<int>(e.getKeyframesSize()) - 1);
  CHECK(testedKeyframe.getRelativePosition() == testingPosition);
  CHECK_FALSE(testedKeyframe.getFrequencyModulation().has_value());
  REQUIRE(testedKeyframe.getAmplitudeModulation().has_value());
  CHECK(testedKeyframe.getAmplitudeModulation().value() == Approx(testingAmplitude));
}

TEST_CASE("addKeyframe with frequency", "[addKeyframe]") {
  Effect e(0, 1, BaseSignal::SawToothUp);
  const size_t n = e.getKeyframesSize();

  const int testingPosition = 42;
  const double testingFrequency = 357;
  e.addKeyframe(testingPosition, std::nullopt, testingFrequency);

  REQUIRE(e.getKeyframesSize() == n + 1);
  Keyframe testedKeyframe = e.getKeyframeAt(static_cast<int>(e.getKeyframesSize()) - 1);
  CHECK(testedKeyframe.getRelativePosition() == testingPosition);
  CHECK_FALSE(testedKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(testedKeyframe.getFrequencyModulation().has_value());
  CHECK(testedKeyframe.getFrequencyModulation().value() == testingFrequency);
}

TEST_CASE("addKeyframe with amplitude and frequency", "[addKeyframe]") {
  Effect e(0, 1, BaseSignal::SawToothUp);
  const size_t n = e.getKeyframesSize();

  const int testingPosition = 24;
  const double testingAmplitude = .9543;
  const int testingFrequency = 0;
  e.addKeyframe(testingPosition, testingAmplitude, testingFrequency);

  REQUIRE(e.getKeyframesSize() == n + 1);
  Keyframe testedKeyframe = e.getKeyframeAt(static_cast<int>(e.getKeyframesSize()) - 1);
  CHECK(testedKeyframe.getRelativePosition() == testingPosition);
  REQUIRE(testedKeyframe.getAmplitudeModulation().has_value());
  CHECK(testedKeyframe.getAmplitudeModulation().value() == Approx(testingAmplitude));
  REQUIRE(testedKeyframe.getFrequencyModulation().has_value());
  CHECK(testedKeyframe.getFrequencyModulation().value() == Approx(testingFrequency));
}
