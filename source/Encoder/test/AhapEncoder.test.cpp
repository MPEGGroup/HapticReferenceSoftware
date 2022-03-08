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
  const std::string testingParameterID = "HapticSharpnessControl";
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

  std::vector<std::pair<int, double>> res;
  int resExitCode = AhapEncoder::extractKeyframes(&testingParameterCurve, &res);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == testingControlPoints.size());
  for (size_t i = 0; i < res.size(); i++) {
    CHECK(static_cast<float>(res.at(i).first) ==
          Approx((testingTime + testingControlPoints.at(i).first) * 1000));
    CHECK(res.at(i).second == Approx(testingControlPoints.at(i).second));
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
  const std::string testingParameterID = "HapticSharpnessControl";
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

  std::vector<std::pair<int, double>> res;
  int resExitCode = AhapEncoder::extractKeyframes(&testingParameterCurve, &res);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == testingControlPoints.size());
  for (size_t i = 0; i < res.size(); i++) {
    CHECK(static_cast<float>(res.at(i).first) ==
          Approx((testingTime + testingControlPoints.at(i).first) * 1000));
    CHECK(res.at(i).second == Approx(testingControlPoints.at(i).second));
  }
}

TEST_CASE("extractContinuous without duration", "[extractContinuous]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingTime = 0.015;
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractContinuous without time", "[extractContinuous]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingDuration = 0.25;
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["EventDuration"] = testingDuration;
  testingContinuous["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractContinuous without event parameters", "[extractContinuous]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingTime = 0.015;
  const float testingDuration = 0.25;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventDuration"] = testingDuration;

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractContinuous without event type", "[extractContinuous]") {
  const float testingTime = 0.015;
  const float testingDuration = 0.25;
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventDuration"] = testingDuration;
  testingContinuous["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractContinuous with incorrect event type", "[extractContinuous]") {
  const std::string testingEventType = "HapticTransient";
  const float testingTime = 0.015;
  const float testingDuration = 0.25;
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventDuration"] = testingDuration;
  testingContinuous["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("extractContinuous without modulation function", "[extractContinuous]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingTime = 0.015;
  const float testingDuration = 0.25;
  const float testingIntensity = 1;
  const float testingSharpness = 0;

  const float expectedAmplitude = 0.6138;
  const int expectedFrequency = 65;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventDuration"] = testingDuration;
  testingContinuous["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == 1);
  haptics::types::Effect resEffect = res.at(0);
  CHECK(resEffect.getBaseSignal() == haptics::types::BaseSignal::Sine);
  CHECK(resEffect.getPhase() == Approx(0));
  CHECK(static_cast<float>(resEffect.getPosition()) == Approx(testingTime * 1000));

  REQUIRE(resEffect.getKeyframesSize() == 2);
  haptics::types::Keyframe resKeyframe = resEffect.getKeyframeAt(0);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(resKeyframe.getRelativePosition().value() == 0);
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);

  resKeyframe = resEffect.getKeyframeAt(1);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(static_cast<float>(resKeyframe.getRelativePosition().value()) ==
        Approx(testingDuration * 1000));
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("extractContinuous without modulation function and default values",
          "[extractContinuous]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingTime = 0.015;
  const float testingDuration = 0.25;

  const float expectedAmplitude = 0.5;
  const int expectedFrequency = 90;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventDuration"] = testingDuration;
  testingContinuous["EventParameters"] = nlohmann::json::array();

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == 1);
  haptics::types::Effect resEffect = res.at(0);
  CHECK(resEffect.getBaseSignal() == haptics::types::BaseSignal::Sine);
  CHECK(resEffect.getPhase() == Approx(0));
  CHECK(static_cast<float>(resEffect.getPosition()) == Approx(testingTime * 1000));

  REQUIRE(resEffect.getKeyframesSize() == 2);
  haptics::types::Keyframe resKeyframe = resEffect.getKeyframeAt(0);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(resKeyframe.getRelativePosition().value() == 0);
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);

  resKeyframe = resEffect.getKeyframeAt(1);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(static_cast<float>(resKeyframe.getRelativePosition().value()) ==
        Approx(testingDuration * 1000));
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("extractContinuous with modulation function", "[extractContinuous]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingTime = 0.015;
  const float testingDuration = 0.25;
  const float testingIntensity = 1;
  const float testingSharpness = 0;

  nlohmann::json testingContinuous = nlohmann::json::object();
  testingContinuous["EventType"] = testingEventType;
  testingContinuous["Time"] = testingTime;
  testingContinuous["EventDuration"] = testingDuration;
  testingContinuous["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  const std::vector<std::pair<int, double>> testingAmplitudeModulation = {{10, 0}, {100, 1}};
  const std::vector<std::pair<int, double>> testingFrequencyModulation = {
      {90, 0}, {200, 1}, {300, 0}};

  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractContinuous(
      &testingContinuous, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == 1);
  haptics::types::Effect resEffect = res.at(0);
  CHECK(resEffect.getBaseSignal() == haptics::types::BaseSignal::Sine);
  CHECK(resEffect.getPhase() == Approx(0));
  CHECK(static_cast<float>(resEffect.getPosition()) == Approx(testingTime * 1000));

  REQUIRE(resEffect.getKeyframesSize() == 5);
  haptics::types::Keyframe resKeyframe = resEffect.getKeyframeAt(0);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(resKeyframe.getRelativePosition().value() == 0);
  const float expectedAmplitude_0 = 0.0341;
  const int expectedFrequency_0 = 65;
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude_0));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency_0);

  resKeyframe = resEffect.getKeyframeAt(1);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  CHECK_FALSE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(static_cast<double>(resKeyframe.getRelativePosition().value()) ==
        Approx(testingFrequencyModulation.at(0).first - testingTime * 1000));
  const int expectedFrequency_1 = 65;
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency_1);

  resKeyframe = resEffect.getKeyframeAt(2);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  CHECK_FALSE(resKeyframe.getFrequencyModulation().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  CHECK(static_cast<double>(resKeyframe.getRelativePosition().value()) ==
        Approx(testingAmplitudeModulation.at(1).first - testingTime * 1000));
  const float expectedAmplitude_2 = 0.6138;
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude_2));

  resKeyframe = resEffect.getKeyframeAt(3);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  CHECK_FALSE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(static_cast<double>(resKeyframe.getRelativePosition().value()) ==
        Approx(testingFrequencyModulation.at(1).first - testingTime * 1000));
  const int expectedFrequency_3 = 300;
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency_3);

  resKeyframe = resEffect.getKeyframeAt(4);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  CHECK(static_cast<double>(resKeyframe.getRelativePosition().value()) ==
        Approx(testingDuration * 1000));
  const float expectedAmplitude_4 = 0.6138;
  const int expectedFrequency_4 = 223;
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude_4));
  CHECK(resKeyframe.getFrequencyModulation().value() == Approx(expectedFrequency_4));
}

TEST_CASE("extractTransients without time", "[extractTransients]") {
  const std::string testingEventType = "HapticTransient";
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["EventType"] = testingEventType;
  testingTransient["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractTransients without event parameters", "[extractTransients]") {
  const std::string testingEventType = "HapticTransient";
  const float testingTime = 0.015;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["EventType"] = testingEventType;
  testingTransient["Time"] = testingTime;

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractTransients without event type", "[extractTransients]") {
  const float testingTime = 0.015;
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["Time"] = testingTime;
  testingTransient["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

TEST_CASE("extractTransients with incorrect event type", "[extractTransients]") {
  const std::string testingEventType = "HapticContinuous";
  const float testingTime = 0.015;
  const float testingIntensity = 0.8;
  const float testingSharpness = 0.4;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["EventType"] = testingEventType;
  testingTransient["Time"] = testingTime;
  testingTransient["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_FAILURE);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("extractTransients without modulation function", "[extractTransients]") {
  const std::string testingEventType = "HapticTransient";
  const float testingTime = 0.015;
  const float testingIntensity = 1;
  const float testingSharpness = 0.5;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["EventType"] = testingEventType;
  testingTransient["Time"] = testingTime;
  testingTransient["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == 1);
  haptics::types::Effect resEffect = res.at(0);
  CHECK(static_cast<float>(resEffect.getPosition()) == Approx(testingTime * 1000));

  REQUIRE(resEffect.getKeyframesSize() == 1);
  haptics::types::Keyframe resKeyframe = resEffect.getKeyframeAt(0);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(resKeyframe.getRelativePosition().value() == 0);
  const float expectedAmplitude = 0.792;
  const int expectedFrequency = 90;
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("extractTransients without modulation function and default values",
          "[extractTransients]") {
  const std::string testingEventType = "HapticTransient";
  const float testingTime = 0.015;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["EventType"] = testingEventType;
  testingTransient["Time"] = testingTime;
  testingTransient["EventParameters"] = nlohmann::json::array();

  std::vector<std::pair<int, double>> testingAmplitudeModulation;
  std::vector<std::pair<int, double>> testingFrequencyModulation;
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == 1);
  haptics::types::Effect resEffect = res.at(0);
  CHECK(static_cast<float>(resEffect.getPosition()) == Approx(testingTime * 1000));

  REQUIRE(resEffect.getKeyframesSize() == 1);
  haptics::types::Keyframe resKeyframe = resEffect.getKeyframeAt(0);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(resKeyframe.getRelativePosition().value() == 0);
  const float expectedAmplitude = 0.5;
  const int expectedFrequency = 90;
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("extractTransients with modulation function", "[extractTransients]") {
  const std::string testingEventType = "HapticTransient";
  const float testingTime = 0.015;
  const float testingIntensity = 1;
  const float testingSharpness = 0.5;

  nlohmann::json testingTransient = nlohmann::json::object();
  testingTransient["EventType"] = testingEventType;
  testingTransient["Time"] = testingTime;
  testingTransient["EventParameters"] =
      (nlohmann::json::array)({{{"ParameterID", "HapticIntensity"},
                                {"ParameterValue", testingIntensity}},
                               {{"ParameterID", "HapticSharpness"},
                                {"ParameterValue", testingSharpness}}});

  const std::vector<std::pair<int, double>> testingAmplitudeModulation = {{10, 0}, {100, 0.5}};
  const std::vector<std::pair<int, double>> testingFrequencyModulation = {};
  std::vector<haptics::types::Effect> res;
  int resExitCode = AhapEncoder::extractTransients(
      &testingTransient, &res, &testingAmplitudeModulation, &testingFrequencyModulation);

  REQUIRE(resExitCode == EXIT_SUCCESS);
  REQUIRE(res.size() == 1);
  haptics::types::Effect resEffect = res.at(0);
  CHECK(static_cast<float>(resEffect.getPosition()) == Approx(testingTime * 1000));

  REQUIRE(resEffect.getKeyframesSize() == 1);
  haptics::types::Keyframe resKeyframe = resEffect.getKeyframeAt(0);
  REQUIRE(resKeyframe.getRelativePosition().has_value());
  REQUIRE(resKeyframe.getAmplitudeModulation().has_value());
  REQUIRE(resKeyframe.getFrequencyModulation().has_value());
  CHECK(resKeyframe.getRelativePosition().value() == 0);
  const float expectedAmplitude = 0.022;
  const int expectedFrequency = 90;
  CHECK(resKeyframe.getAmplitudeModulation().value() == Approx(expectedAmplitude));
  CHECK(resKeyframe.getFrequencyModulation().value() == expectedFrequency);
}
