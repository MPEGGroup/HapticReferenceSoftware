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

#include <Encoder/include/AhapEncoder.h>
#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

using haptics::encoder::AhapEncoder;

TEST_CASE("extractKeyframes with ParameterCurveControlPoints not set", "[extractKeyframes]") {
  const std::string testingParameterID = "HapticIntensityControl";
  const float testingTime = 42.358;

  nlohmann::json testingParameterCurve = nlohmann::json::object();
  testingParameterCurve["ParameterID"] = testingParameterID;
  testingParameterCurve["Time"] = testingTime;

  std::vector<std::pair<int, double>> resKeyframes;
  int res = AhapEncoder::extractKeyframes(&testingParameterCurve, &resKeyframes);

  REQUIRE(res == EXIT_FAILURE);
}

TEST_CASE("extractKeyframes with empty ParameterCurveControlPoints", "[extractKeyframes]") {
  const std::string testingParameterID = "HapticIntensityControl";
  const float testingTime = 42.358;

  nlohmann::json testingParameterCurve = nlohmann::json::object();
  testingParameterCurve["ParameterID"] = testingParameterID;
  testingParameterCurve["Time"] = testingTime;
  testingParameterCurve["ParameterCurveControlPoints"] = nlohmann::json::array();

  std::vector<std::pair<int, double>> resKeyframes;
  int res = AhapEncoder::extractKeyframes(&testingParameterCurve, &resKeyframes);

  REQUIRE(res == EXIT_SUCCESS);
  CHECK(resKeyframes.empty());
}

TEST_CASE("extractKeyframes with ParameterCurveControlPoints", "[extractKeyframes]") {
  const std::string testingParameterID = "HapticIntensityControl";
  const float testingTime = 42.358;
  const std::vector<std::pair<float, float>> testingControlPoints = {{35, .05}, {-5.2, 114.05}};

  nlohmann::json testingParameterCurve = nlohmann::json::object();
  testingParameterCurve["ParameterID"] = testingParameterID;
  testingParameterCurve["Time"] = testingTime;
  testingParameterCurve["ParameterCurveControlPoints"] = nlohmann::json::array();
  for (auto controlPoint : testingControlPoints) {
    testingParameterCurve["ParameterCurveControlPoints"].push_back(
        {{"Time", controlPoint.first}, {"ParameterValue", controlPoint.second}});
  }

  std::vector<std::pair<int, double>> resKeyframes;
  int res = AhapEncoder::extractKeyframes(&testingParameterCurve, &resKeyframes);

  REQUIRE(res == EXIT_SUCCESS);
  CHECK(resKeyframes.size() == testingControlPoints.size());
  for (size_t i = 0; i < resKeyframes.size(); i++) {
    CHECK(static_cast<float>(resKeyframes.at(i).first) ==
          Approx((testingTime + testingControlPoints.at(i).first) * 1000));
    CHECK(resKeyframes.at(i).second == Approx(testingControlPoints.at(i).second));
  }
}

TEST_CASE("extractKeyframes with incorrect ParameterCurveControlPoints", "[extractKeyframes]") {
  const std::string testingParameterID = "HapticIntensityControl";
  const float testingTime = 42.358;
  const std::vector<std::pair<float, float>> testingControlPoints = {{35, .05}, {-5.2, 114.05}};

  nlohmann::json testingParameterCurve = nlohmann::json::object();
  testingParameterCurve["ParameterID"] = testingParameterID;
  testingParameterCurve["Time"] = testingTime;
  testingParameterCurve["ParameterCurveControlPoints"] = nlohmann::json::object();
  testingParameterCurve["ParameterCurveControlPoints"]["Time"] = 0;
  testingParameterCurve["ParameterCurveControlPoints"]["ParameterValue"] = 0;

  std::vector<std::pair<int, double>> resKeyframes;
  int res = AhapEncoder::extractKeyframes(&testingParameterCurve, &resKeyframes);

  REQUIRE(res == EXIT_FAILURE);
}

TEST_CASE("extractKeyframes with incorrect ControlPoints", "[extractKeyframes]") {
  const std::string testingParameterID = "HapticIntensityControl";
  const float testingTime = 42.358;
  const std::vector<std::pair<float, float>> testingControlPoints = {{}};
  const std::vector<float> testingIncorrectTime = {4536};
  const nlohmann::json testingIncorrectParameterValue = nlohmann::json::object();

  nlohmann::json testingParameterCurve = nlohmann::json::object();
  testingParameterCurve["ParameterID"] = testingParameterID;
  testingParameterCurve["Time"] = testingTime;
  testingParameterCurve["ParameterCurveControlPoints"] = nlohmann::json::array();
  testingParameterCurve["ParameterCurveControlPoints"].push_back(
      {{"Time", testingControlPoints.at(0).first}});
  for (auto controlPoint : testingControlPoints) {
    testingParameterCurve["ParameterCurveControlPoints"].push_back(
        {{"Time", controlPoint.first}, {"ParameterValue", controlPoint.second}});
  }
  testingParameterCurve["ParameterCurveControlPoints"].push_back(
      {{"Time", testingIncorrectTime}, {"ParameterValue", testingIncorrectParameterValue}});
  testingParameterCurve["ParameterCurveControlPoints"].push_back(
      {{"ParameterValue", testingControlPoints.at(0).second}});

  std::vector<std::pair<int, double>> resKeyframes;
  int res = AhapEncoder::extractKeyframes(&testingParameterCurve, &resKeyframes);

  REQUIRE(res == EXIT_SUCCESS);
  CHECK(resKeyframes.size() == testingControlPoints.size());
  for (size_t i = 0; i < resKeyframes.size(); i++) {
    CHECK(static_cast<float>(resKeyframes.at(i).first) ==
          Approx((testingTime + testingControlPoints.at(i).first) * 1000));
    CHECK(resKeyframes.at(i).second == Approx(testingControlPoints.at(i).second));
  }
}
