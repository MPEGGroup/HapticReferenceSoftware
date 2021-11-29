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

#include "../../tools/include/Point.h"
#include "../include/PcmEncoder.h"

using haptics::encoder::PcmEncoder;
using haptics::tools::Point;

TEST_CASE("localExtrema with empty input and no border", "[localExtrema][empty][withoutBorder]") {
  std::vector<Point> res = PcmEncoder::localExtrema(std::vector<int16_t>{}, false);

  CHECK(res.empty());
}

TEST_CASE("localExtrema with empty input and border", "[localExtrema][empty][withBorder]") {
  std::vector<Point> res = PcmEncoder::localExtrema(std::vector<int16_t>{}, true);

  CHECK(res.empty());
}

TEST_CASE("localExtrema with entry of 1 point",
          "[localExtrema][1_point][withBorder][withoutBorder]") {
  const std::vector<int16_t> testingValues = {0, -1, 42, 4153, 346, -5423, 6, 1};

  int16_t i = 0;
  for (int16_t testingElement : testingValues) {
    std::vector<int16_t> v = {testingElement};

    DYNAMIC_SECTION("1 point entry without border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<Point> res = PcmEncoder::localExtrema(v, false);

      CHECK(res.empty());
    }

    DYNAMIC_SECTION("1 point entry with border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<Point> res = PcmEncoder::localExtrema(v, true);

      REQUIRE_FALSE(res.empty());
      int16_t pi = 0;
      Point p1(pi, testingElement);
      Point p2(pi, testingElement);
      REQUIRE(res.size() == 2);
      REQUIRE((res[0] == p1));
      REQUIRE((res[1] == p2));
    }
    i++;
  }
}

TEST_CASE("localExtrema with entry of 2 point",
          "[localExtrema][2_points][withBorder][withoutBorder]") {
  const std::vector<std::vector<int16_t>> testingValues = {
      {1, 1}, {0, 1}, {-5432, 42}, {-1, -3}, {6542, 3461}};

  int16_t i = 0;
  for (const auto &v : testingValues) {
    DYNAMIC_SECTION("2 points entry without border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<Point> res = PcmEncoder::localExtrema(v, false);

      CHECK(res.empty());
    }

    DYNAMIC_SECTION("2 points entry with border (TESTING CASE: " + std::to_string(i) + ")") {
      std::vector<Point> res = PcmEncoder::localExtrema(v, true);

      REQUIRE_FALSE(res.empty());
      int16_t pi = 0;
      Point p1(pi, v[pi]);
      pi = 1;
      Point p2(pi, v[pi]);
      REQUIRE(res.size() == 2);
      REQUIRE((res[0] == p1));
      REQUIRE((res[1] == p2));
    }
    i++;
  }
}

TEST_CASE("localExtrema without border every extremum are extracted",
          "[localExtrema][N_points][withoutBorder]") {
  const std::vector<int16_t> testingValues = {0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 2,
                                              3, 4, 6, 8, 7, 8, 6, 3, 1, 0};

  std::vector<Point> res = PcmEncoder::localExtrema(testingValues, false);

  REQUIRE_FALSE(res.empty());

  bool isMaximum = false;
  bool isMinimum = false;
  bool isFlat = false;
  int16_t i = 1;
  int16_t previousValue = 0;
  int16_t currentValue = testingValues[i - 1];
  int16_t nextValue = testingValues[i];
  for (; i < int16_t(testingValues.size() - 1); i++) {
    previousValue = currentValue;
    currentValue = nextValue;
    nextValue = testingValues[i + 1];
    isFlat = previousValue == currentValue && currentValue == nextValue;
    isMaximum = previousValue <= currentValue && currentValue >= nextValue;
    isMinimum = previousValue >= currentValue && currentValue <= nextValue;

    if (!isFlat && (isMaximum || isMinimum)) {
      Point p(i, testingValues[i]);
      bool contain = false;
      for (Point element : res) {
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
  const std::vector<int16_t> testingValues = {0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 2,
                                              3, 4, 6, 8, 7, 8, 6, 3, 1, 0};

  std::vector<Point> res = PcmEncoder::localExtrema(testingValues, false);

  REQUIRE_FALSE(res.empty());

  bool isMaximum = false;
  bool isMinimum = false;
  bool isFlat = false;
  int16_t i = 1;
  int16_t previousValue = 0;
  int16_t currentValue = testingValues[i - 1];
  int16_t nextValue = testingValues[i];
  auto testingValuesSize = int16_t(testingValues.size());
  for (Point extremumElement : res) {
    previousValue = testingValues[extremumElement.x - 1];
    currentValue = testingValues[extremumElement.x];
    nextValue = testingValues[extremumElement.x + 1];

    CHECK(currentValue == extremumElement.y);

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
  const std::vector<int16_t> testingValues = {0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 2,
                                              3, 4, 6, 8, 7, 8, 6, 3, 1, 0};
  std::vector<Point> res = PcmEncoder::localExtrema(testingValues, true);
  auto resSize = int16_t(res.size());

  REQUIRE(resSize >= 2);

  SECTION("first and last values are the borders") {
    Point first = res[0];
    Point last = res[resSize - 1];
    auto testingValuesSize = int16_t(testingValues.size());

    CHECK(first.x == 0);
    CHECK(first.y == testingValues[0]);

    CHECK(last.x == testingValuesSize - 1);
    CHECK(last.y == testingValues[testingValuesSize - 1]);
  }

  SECTION("The rest is the same values as without border") {
    std::vector<Point> resWithoutBorder = PcmEncoder::localExtrema(testingValues, false);
    std::vector<Point> subRes(res.begin() + 1, res.begin() + resSize - 1);

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
