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

#include <Encoder/include/PcmEncoder.h>
#include <catch2/catch.hpp>

using haptics::encoder::PcmEncoder;
using haptics::types::Band;
using haptics::types::BandType;
using haptics::types::Effect;
using haptics::types::EncodingModality;
using haptics::types::Keyframe;

TEST_CASE("localExtrema with empty input and no border", "[localExtrema][empty][withoutBorder]") {
  std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(std::vector<double>{}, false);

  CHECK(res.empty());
}

TEST_CASE("localExtrema with empty input and border", "[localExtrema][empty][withBorder]") {
  std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(std::vector<double>{}, true);

  CHECK(res.empty());
}

TEST_CASE("localExtrema with entry of 1 point",
          "[localExtrema][1_point][withBorder][withoutBorder]") {
  const std::vector<double> testingValues = {0, -1, 42, 4153, 346, -5423, 6, 1};

  int i = 0;
  for (double testingElement : testingValues) {
    std::vector<double> v = {testingElement};

    DYNAMIC_SECTION("1 point entry without border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(v, false);

      CHECK(res.empty());
    }

    DYNAMIC_SECTION("1 point entry with border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(v, true);

      REQUIRE_FALSE(res.empty());
      int pi = 0;
      std::pair<int, double> p1(pi, testingElement);
      std::pair<int, double> p2(pi, testingElement);
      REQUIRE(res.size() == 2);
      REQUIRE((res[0] == p1));
      REQUIRE((res[1] == p2));
    }
    i++;
  }
}

TEST_CASE("localExtrema with entry of 2 point",
          "[localExtrema][2_points][withBorder][withoutBorder]") {
  const std::vector<std::vector<double>> testingValues = {
      {1, 1}, {0, 1}, {-5432, 42}, {-1, -3}, {6542, 3461}};

  int i = 0;
  for (const auto &v : testingValues) {
    DYNAMIC_SECTION("2 points entry without border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(v, false);

      CHECK(res.empty());
    }

    DYNAMIC_SECTION("2 points entry with border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(v, true);

      REQUIRE_FALSE(res.empty());
      int pi = 0;
      std::pair<int, double> p1(pi, v[pi]);
      pi = 1;
      std::pair<int, double> p2(pi, v[pi]);
      REQUIRE(res.size() == 2);
      REQUIRE((res[0] == p1));
      REQUIRE((res[1] == p2));
    }
    i++;
  }
}

TEST_CASE("localExtrema without border every extremum are extracted",
          "[localExtrema][N_points][withoutBorder]") {
  const std::vector<double> testingValues = {0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 2,
                                             3, 4, 6, 8, 7, 8, 6, 3, 1, 0};

  std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(testingValues, false);

  REQUIRE_FALSE(res.empty());

  bool isMaximum = false;
  bool isMinimum = false;
  bool isFlat = false;
  int i = 1;
  double previousValue = 0;
  double currentValue = testingValues[i - 1];
  double nextValue = testingValues[i];
  for (; i < int(testingValues.size() - 1); i++) {
    previousValue = currentValue;
    currentValue = nextValue;
    nextValue = testingValues[i + 1];
    isFlat = previousValue == currentValue && currentValue == nextValue;
    isMaximum = previousValue <= currentValue && currentValue >= nextValue;
    isMinimum = previousValue >= currentValue && currentValue <= nextValue;

    if (!isFlat && (isMaximum || isMinimum)) {
      std::pair<int, double> p(i, testingValues[i]);
      bool contain = false;
      for (std::pair<int, double> element : res) {
        if (element == p) {
          contain = true;
          break;
        }
      }
      CHECK(contain);
    }
  }
}

TEST_CASE("localExtrema without border extract only extremum",
          "[localExtrema][N_points][withoutBorder]") {
  const std::vector<double> testingValues = {0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 2,
                                             3, 4, 6, 8, 7, 8, 6, 3, 1, 0};

  std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(testingValues, false);

  REQUIRE_FALSE(res.empty());

  bool isMaximum = false;
  bool isMinimum = false;
  bool isFlat = false;
  int i = 1;
  double previousValue = 0;
  double currentValue = testingValues[i - 1];
  double nextValue = testingValues[i];
  for (std::pair<int, double> extremumElement : res) {
    previousValue = testingValues[extremumElement.first - 1];
    currentValue = testingValues[extremumElement.first];
    nextValue = testingValues[extremumElement.first + 1];

    CHECK(currentValue == Approx(extremumElement.second));

    isFlat = previousValue == currentValue && currentValue == nextValue;
    isMaximum = previousValue <= currentValue && currentValue >= nextValue;
    isMinimum = previousValue >= currentValue && currentValue <= nextValue;

    CHECK_FALSE(isFlat);
    CHECK((isMaximum || isMinimum));
  }
}

TEST_CASE("localExtrema with border extract values like without border plus first and last indexes "
          "are the borders",
          "[localExtrema][N_points][withBorder]") {
  const std::vector<double> testingValues = {0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 2,
                                             3, 4, 6, 8, 7, 8, 6, 3, 1, 0};
  std::vector<std::pair<int, double>> res = PcmEncoder::localExtrema(testingValues, true);
  auto resSize = int(res.size());

  REQUIRE(resSize >= 2);

  SECTION("first and last values are the borders") {
    std::pair<int, double> first = res[0];
    std::pair<int, double> last = res[resSize - 1];
    auto testingValuesSize = int(testingValues.size());

    CHECK(first.first == 0);
    CHECK(first.second == Approx(testingValues[0]));

    CHECK(last.first == testingValuesSize - 1);
    CHECK(last.second == Approx(testingValues[testingValuesSize - 1]));
  }

  SECTION("The rest is the same values as without border") {
    std::vector<std::pair<int, double>> resWithoutBorder =
        PcmEncoder::localExtrema(testingValues, false);
    std::vector<std::pair<int, double>> subRes(res.begin() + 1, res.begin() + resSize - 1);

    REQUIRE(subRes.size() == resWithoutBorder.size());

    auto subResIt = subRes.begin();
    auto resWithoutBorderIt = resWithoutBorder.begin();
    while (subResIt != subRes.end() && resWithoutBorderIt != resWithoutBorder.end()) {
      CHECK(((*subResIt) == (*resWithoutBorderIt)));

      subResIt++;
      resWithoutBorderIt++;
    }
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("convertToCurveBand", "[convertToCurveBand]") {
  std::vector<std::pair<int, double>> testingPoints = {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
      {0, .654}, {24, 1}, {40, 0}, {80, -.34}, {656, -.5648}, {2500, .15},
  };
  const double testingSamplerate = 8000;
  const double testingCurveFrequencyLimit = 72;
  const std::vector<double> expectedTimes = {0, 3, 5, 10, 82, 312};
  REQUIRE(testingPoints.size() == expectedTimes.size());

  Band res;
  REQUIRE(PcmEncoder::convertToCurveBand(testingPoints, testingSamplerate,
                                         testingCurveFrequencyLimit, &res));
  CHECK(res.getBandType() == BandType::Curve);
  CHECK(res.getEncodingModality() == EncodingModality::Quantized);
  CHECK(res.getWindowLength() == 0);
  CHECK(res.getLowerFrequencyLimit() == 0);
  CHECK(res.getUpperFrequencyLimit() == testingCurveFrequencyLimit);
  REQUIRE(res.getEffectsSize() == 1);
  Effect e = res.getEffectAt(0);
  REQUIRE(e.getKeyframesSize() == testingPoints.size());
  int i = 0;
  Keyframe k;
  for (std::pair<int, double> testingP : testingPoints) {
    k = e.getKeyframeAt(i);
    CHECK(k.getRelativePosition() == expectedTimes[i]);
    CHECK_FALSE(k.getFrequencyModulation().has_value());
    CHECK(k.getAmplitudeModulation().has_value());
    CHECK(k.getAmplitudeModulation().value() == Approx(testingP.second));
    i++;
  }
}