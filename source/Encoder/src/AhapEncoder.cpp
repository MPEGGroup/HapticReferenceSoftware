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
#include <Tools/include/Tools.h>
#include <algorithm>
#include <cmath>
#include <fstream>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26812)
#endif
#include <rapidjson/istreamwrapper.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// const double SEC_TO_MSEC = 1000.0;
const float DEFAULT_AMPLITUDE = 0.5;
const int DEFAULT_FREQUENCY = 90;

const double BASE_AMPLITUDE_MIN = 0;
const double BASE_AMPLITUDE_MAX = 1;
const double ACTUAL_AMPLITUDE_MIN = 0;
const double ACTUAL_AMPLITUDE_MIN_C = 0.6138;
const double ACTUAL_AMPLITUDE_MIN_T = 0.792;

const double BASE_FREQUENCY_MIN = 0;
const double BASE_FREQUENCY_MAX = 1;
const int ACTUAL_FREQUENCY_MIN = 65;
const int ACTUAL_FREQUENCY_MAX = 300;

namespace haptics::encoder {

[[nodiscard]] auto AhapEncoder::encode(std::string &filename, haptics::types::Perception &out,
                                       const unsigned int timescale) -> int {
  if (out.getChannelsSize() > 1) {
    return EXIT_FAILURE;
  }

  std::ifstream ifs(filename);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document json;
  if (json.ParseStream<rapidjson::kParseTrailingCommasFlag>(isw).HasParseError()) {
    std::cerr << "Invalid AHAP input file: JSON parsing error" << std::endl;
    return EXIT_FAILURE;
  }
  if (!json.IsObject()) {
    std::cerr << "Invalid AHAP input file: not a JSON object" << std::endl;
    return EXIT_FAILURE;
  }
  if (!json.HasMember("Pattern") || !json["Pattern"].IsArray()) {
    std::cerr << "Invalid AHAP input file: missing or invalid Pattern" << std::endl;
    return EXIT_FAILURE;
  }

  auto pattern = json["Pattern"].GetArray();

  std::vector<std::pair<int, double>> amplitudes;
  std::vector<std::pair<int, double>> frequencies;
  std::vector<haptics::types::Effect> continuous;
  std::vector<haptics::types::Effect> transients;

  int ret = 0;

  // FOR LOOP ON KEYFRAMES
  for (auto &e : pattern) {
    if (e.HasMember("ParameterCurve")) {
      if (e["ParameterCurve"]["ParameterID"] == "HapticIntensityControl") {
        ret = extractKeyframes(e["ParameterCurve"].GetObject(), &amplitudes, timescale);
        if (ret != 0) {
          std::cerr << "ERROR IN AMPLITUDE EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }

      if (e["ParameterCurve"]["ParameterID"] == "HapticSharpnessControl") {
        ret = extractKeyframes(e["ParameterCurve"].GetObject(), &frequencies, timescale);
        if (ret != 0) {
          std::cerr << "ERROR IN FREQUENCY EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }

  for (auto &e : pattern) {

    if (e.HasMember("Event")) {
      if (e["Event"]["EventType"] == "HapticTransient") {
        ret = extractTransients(e["Event"].GetObject(), &transients, &amplitudes, &frequencies,
                                timescale);
        if (ret != 0) {
          std::cerr << "ERROR IN TRANSIENT EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }

      if (e["Event"]["EventType"] == "HapticContinuous") {
        ret = extractContinuous(e["Event"].GetObject(), &continuous, &amplitudes, &frequencies,
                                timescale);
        if (ret != 0) {
          std::cerr << "ERROR IN CONTINUOUS EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }

  haptics::types::Channel myChannel(0, "placeholder description", 1, 1, 0);
  if (out.getChannelsSize() == 0) {
    out.addChannel(myChannel);
  }
  myChannel = out.getChannelAt(0);

  // TRANSIENTS
  types::Band myBand =
      types::Band(types::BandType::Transient, MIN_AHAP_FREQUENCY, MAX_AHAP_FREQUENCY);
  types::Effect transientEffect;
  for (types::Effect e : transients) {
    auto pos = e.getPosition();
    for (int i = 0; i < static_cast<int>(e.getKeyframesSize()); i++) {
      auto keyframe = e.getKeyframeAt(i);
      keyframe.setRelativePosition(keyframe.getRelativePosition().value_or(0) + pos);
      transientEffect.addKeyframe(keyframe);
    }
  }
  myBand.addEffect(transientEffect);
  if (myBand.getEffectsSize() > 0) {
    myChannel.addBand(myBand);
  }

  // CONTINUOUS
  types::Band *b = nullptr;
  for (types::Effect e : continuous) {
    b = myChannel.findBandAvailable(
        e.getPosition(),
        e.getKeyframeAt(static_cast<int>(e.getKeyframesSize()) - 1).getRelativePosition().value(),
        types::BandType::VectorialWave);
    if (b == nullptr) {
      b = myChannel.generateBand(haptics::types::BandType::VectorialWave, MIN_AHAP_FREQUENCY,
                                 MAX_AHAP_FREQUENCY);
    }
    b->addEffect(e);
  }

  out.replaceChannelAt(0, myChannel);
  return EXIT_SUCCESS;
}

[[nodiscard]] auto AhapEncoder::extractKeyframes(const rapidjson::Value::Object &parameterCurve,
                                                 std::vector<std::pair<int, double>> *keyframes,
                                                 const unsigned int timescale) -> int {
  if (!parameterCurve.HasMember("Time") || !parameterCurve["Time"].IsNumber() ||
      !parameterCurve.HasMember("ParameterCurveControlPoints") ||
      !parameterCurve["ParameterCurveControlPoints"].IsArray()) {
    return EXIT_FAILURE;
  }

  for (auto &kahap : parameterCurve["ParameterCurveControlPoints"].GetArray()) {
    if (!kahap.HasMember("Time") || !kahap["Time"].IsNumber() ||
        !kahap.HasMember("ParameterValue") || !kahap["ParameterValue"].IsNumber()) {
      continue;
    }

    std::pair<int, double> k;
    // TIME + curve offset

    k.first = static_cast<int>((kahap["Time"].GetDouble() + parameterCurve["Time"].GetDouble()) *
                               (double)timescale);
    // VALUE
    k.second = kahap["ParameterValue"].GetDouble();

    keyframes->push_back(k);
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto AhapEncoder::extractTransients(
    const rapidjson::Value::Object &event, std::vector<haptics::types::Effect> *transients,
    const std::vector<std::pair<int, double>> *amplitudes,
    const std::vector<std::pair<int, double>> *frequencies, const unsigned int timescale) -> int {
  if (!event.HasMember("Time") || !event["Time"].IsNumber() || !event.HasMember("EventType") ||
      !event["EventType"].IsString() ||
      std::string(event["EventType"].GetString()) != "HapticTransient" ||
      !event.HasMember("EventParameters") || !event["EventParameters"].IsArray()) {
    return EXIT_FAILURE;
  }

  haptics::types::Effect t =
      haptics::types::Effect(static_cast<int>(std::round(event["Time"].GetDouble() * timescale)),
                             0, // modif to get from sec2ms to ticks
                             haptics::types::BaseSignal::Sine, haptics::types::EffectType::Basis);

  haptics::types::Keyframe k;
  k.setAmplitudeModulation(DEFAULT_AMPLITUDE);
  k.setFrequencyModulation(DEFAULT_FREQUENCY);

  // SET VALUES AS DEFINED
  for (auto &param : event["EventParameters"].GetArray()) {
    if (param["ParameterID"] == "HapticIntensity") {
      double amp =
          tools::genericNormalization(BASE_AMPLITUDE_MIN, BASE_AMPLITUDE_MAX, ACTUAL_AMPLITUDE_MIN,
                                      ACTUAL_AMPLITUDE_MIN_T, param["ParameterValue"].GetDouble());
      k.setAmplitudeModulation(static_cast<float>(amp));
    }

    if (param["ParameterID"] == "HapticSharpness") {
      k.setFrequencyModulation(DEFAULT_FREQUENCY);
    }
  }

  // SET MODULATED VALUES IF APPLICABLE
  if (!amplitudes->empty()) {
    // FIND FIRST KEYFRAME AFTER THE EFFECT
    auto first_kf_a =
        std::find_if(amplitudes->begin(), amplitudes->end(),
                     [t](std::pair<int, double> a) { return a.first >= t.getPosition(); });

    // IF NOT FIRST KEY FRAME && THERE IS A KEYFRAME && KEYFRAME IS NOT THE LAST ONE AND ON THE
    // EVENT BEGINNING
    if (first_kf_a > amplitudes->begin() && first_kf_a < amplitudes->end()) {
      // MULTIPLY AMPLITUDE MODULATION
      k.setAmplitudeModulation(static_cast<float>(haptics::tools::linearInterpolation(
                                   *(first_kf_a - 1), *first_kf_a, t.getPosition())) *
                               k.getAmplitudeModulation().value());
    } else if (first_kf_a == amplitudes->begin()) {
      k.setAmplitudeModulation(static_cast<float>(amplitudes->front().second) *
                               k.getAmplitudeModulation().value());
    }
  }

  if (!frequencies->empty()) {
    auto first_kf_f =
        std::find_if(frequencies->begin(), frequencies->end(),
                     [t](std::pair<int, double> f) { return f.first >= t.getPosition(); });

    if (first_kf_f != frequencies->begin() && first_kf_f < frequencies->end()) {
      // CONVERT FREQUENCY VALUE
      double f =
          haptics::tools::linearInterpolation(*(first_kf_f - 1), *first_kf_f, t.getPosition());
      int freq = static_cast<int>(haptics::tools::genericNormalization(
          BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX, f));
      // SUM FREQUENCY MODULATION
      k.setFrequencyModulation(freq + k.getFrequencyModulation().value());
    } else if (first_kf_f == frequencies->begin()) {
      k.setFrequencyModulation(ACTUAL_FREQUENCY_MAX);
    }
  }

  t.addKeyframe(k);

  transients->push_back(t);

  return EXIT_SUCCESS;
}

[[nodiscard]] auto AhapEncoder::extractContinuous(
    const rapidjson::Value::Object &event, std::vector<haptics::types::Effect> *continuous,
    const std::vector<std::pair<int, double>> *amplitudes,
    const std::vector<std::pair<int, double>> *frequencies, const unsigned int timescale) -> int {
  if (!event.HasMember("Time") || !event["Time"].IsNumber() || !event.HasMember("EventType") ||
      !event["EventType"].IsString() ||
      std::string(event["EventType"].GetString()) != "HapticContinuous" ||
      !event.HasMember("EventDuration") || !event["EventDuration"].IsNumber() ||
      !event.HasMember("EventParameters") || !event["EventParameters"].IsArray()) {
    return EXIT_FAILURE;
  }

  haptics::types::Effect c =
      haptics::types::Effect(static_cast<int>(std::round(event["Time"].GetDouble() * timescale)), 0,
                             haptics::types::BaseSignal::Sine, haptics::types::EffectType::Basis);

  haptics::types::Keyframe k_start;
  haptics::types::Keyframe k_end;
  k_start.setAmplitudeModulation(DEFAULT_AMPLITUDE);
  k_start.setFrequencyModulation(DEFAULT_FREQUENCY);
  k_end.setAmplitudeModulation(DEFAULT_AMPLITUDE);
  k_end.setFrequencyModulation(DEFAULT_FREQUENCY);
  double base_freq = BASE_FREQUENCY_MAX;

  k_end.setRelativePosition(
      static_cast<int>(std::round(event["EventDuration"].GetDouble() * timescale)));

  // SET VALUES AS DEFINED
  for (auto &param : event["EventParameters"].GetArray()) {
    if (param["ParameterID"] == "HapticIntensity") {
      double amp =
          tools::genericNormalization(BASE_AMPLITUDE_MIN, BASE_AMPLITUDE_MAX, ACTUAL_AMPLITUDE_MIN,
                                      ACTUAL_AMPLITUDE_MIN_C, param["ParameterValue"].GetDouble());
      k_start.setAmplitudeModulation(static_cast<float>(amp));
      k_end.setAmplitudeModulation(k_start.getAmplitudeModulation());
    }

    if (param["ParameterID"] == "HapticSharpness") {
      base_freq = param["ParameterValue"].GetDouble();
      double freq =
          tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN,
                                      ACTUAL_FREQUENCY_MAX, base_freq);
      k_start.setFrequencyModulation(static_cast<int>(freq));
      k_end.setFrequencyModulation(k_start.getFrequencyModulation());
    }
  }
  c.addKeyframe(k_start);

  // SET MODULATED VALUES IF APPLICABLE
  AhapEncoder::modulateContinuousOnAmplitude(amplitudes, c, k_start, k_end);
  AhapEncoder::modulateContinuousOnFrequency(frequencies, c, k_end, base_freq);

  c.addKeyframe(k_end);

  continuous->push_back(c);

  return EXIT_SUCCESS;
}

auto AhapEncoder::modulateContinuousOnAmplitude(
    const std::vector<std::pair<int, double>> *amplitudes, types::Effect &continuous,
    const Keyframe &firstKeyframe, Keyframe &lastKeyframe) -> void {
  if (amplitudes->empty()) {
    return;
  }

  float base_amp = firstKeyframe.getAmplitudeModulation().value();
  // FIND FIRST KEYFRAME AFTER THE EFFECT
  auto first_kf_a =
      std::find_if(amplitudes->begin(), amplitudes->end(), [continuous](std::pair<int, double> a) {
        return a.first >= continuous.getPosition();
      });

  // IF FIRST KEYFRAME ON START OF EVENT, UPDATE EVENT
  if (first_kf_a->first == continuous.getPosition() && first_kf_a != amplitudes->end() - 1) {
    continuous.getKeyframeAt(0).setAmplitudeModulation(
        continuous.getKeyframeAt(0).getAmplitudeModulation().value() *
        static_cast<float>(first_kf_a->second));
    first_kf_a++;
  } else if ((continuous.getPosition() < first_kf_a->first) && first_kf_a != amplitudes->begin()) {
    float amp = static_cast<float>(haptics::tools::linearInterpolation(
                    *(first_kf_a - 1), *first_kf_a, continuous.getPosition())) *
                continuous.getKeyframeAt(0).getAmplitudeModulation().value();
    continuous.getKeyframeAt(0).setAmplitudeModulation(amp);
  }

  for (auto it = first_kf_a; it < amplitudes->end(); it++) {
    // IF AFTER OR ON THE END OF THE EFFECT, UPDATE END
    if (it->first >= (continuous.getPosition() + lastKeyframe.getRelativePosition().value())) {
      float amp = static_cast<float>(haptics::tools::linearInterpolation(
          *(it - 1), *it, continuous.getPosition() + lastKeyframe.getRelativePosition().value()));
      lastKeyframe.setAmplitudeModulation(amp * lastKeyframe.getAmplitudeModulation().value());
      break;
    }
    // IF IN THE MIDDLE OF THE EFFECT, ADD KEYFRAME
    if (continuous.getPosition() < it->first &&
        it->first < (continuous.getPosition() + lastKeyframe.getRelativePosition().value())) {
      // IF NO KEYFRAME BEFORE, BEGIN MODULATION
      if (it == amplitudes->begin()) {
        continuous.addAmplitudeAt(base_amp, it->first - continuous.getPosition());
      }
      continuous.addAmplitudeAt(static_cast<float>(it->second) * base_amp,
                                it->first - continuous.getPosition());
      // IF NO KEYFRAME AFTER BUT STILL INSIDE, END MODULATION SWITCH BACK TO BASE AMPLITUDE
      if (it == amplitudes->end() - 1) {
        continuous.addAmplitudeAt(base_amp, it->first - continuous.getPosition());
      }
    }
  }
}

auto AhapEncoder::modulateContinuousOnFrequency(
    const std::vector<std::pair<int, double>> *frequencies, types::Effect &continuous,
    Keyframe &lastKeyframe, double base_freq) -> void {
  if (frequencies->empty()) {
    return;
  }

  // FIND FIRST KEYFRAME AFTER THE EFFECT
  auto first_kf_f = std::find_if(
      frequencies->begin(), frequencies->end(),
      [continuous](std::pair<int, double> a) { return a.first >= continuous.getPosition(); });

  // IF FIRST KEYFRAME ON START OF EVENT, UPDATE EVENT
  if (first_kf_f->first == continuous.getPosition() && first_kf_f != frequencies->end() - 1) {
    double freq_d = first_kf_f->second + base_freq;
    freq_d = std::clamp(freq_d, BASE_AMPLITUDE_MIN, BASE_AMPLITUDE_MAX);
    int freq = static_cast<int>(std::round(
        haptics::tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX,
                                             ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX, freq_d)));
    continuous.getKeyframeAt(0).setFrequencyModulation(freq);
    first_kf_f++;
  } else if ((continuous.getPosition() < first_kf_f->first) && first_kf_f != frequencies->begin()) {
    double freq = haptics::tools::chirpInterpolation(
        (first_kf_f - 1)->first, (first_kf_f)->first, (first_kf_f - 1)->second + base_freq,
        (first_kf_f)->second + base_freq, continuous.getPosition());
    freq = std::clamp(freq, BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX);
    continuous.getKeyframeAt(0).setFrequencyModulation(static_cast<int>(std::round(
        haptics::tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX,
                                             ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX, freq))));
  }

  for (auto it = first_kf_f; it < frequencies->end(); it++) {
    // IF AFTER OR ON THE END OF THE EFFECT, UPDATE END
    if (it->first >= (continuous.getPosition() + lastKeyframe.getRelativePosition().value())) {
      double freq = haptics::tools::chirpInterpolation(
          (it - 1)->first, it->first, (it - 1)->second + base_freq, it->second + base_freq,
          continuous.getPosition() + lastKeyframe.getRelativePosition().value());
      freq = std::clamp(freq + base_freq, BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX);
      lastKeyframe.setFrequencyModulation(static_cast<int>(std::round(
          haptics::tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX,
                                               ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX, freq))));
      break;
    }
    // IF IN THE MIDDLE OF THE EFFECT, ADD KEYFRAME
    if (continuous.getPosition() < it->first &&
        it->first < (continuous.getPosition() + lastKeyframe.getRelativePosition().value())) {
      // IF NO KEYFRAME BEFORE, BEGIN MODULATION
      if (it == frequencies->begin()) {
        int freq = static_cast<int>(std::round(haptics::tools::genericNormalization(
            BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
            base_freq)));
        continuous.addFrequencyAt(freq, it->first - continuous.getPosition());
      }
      double freq_d = it->second + base_freq;
      freq_d = std::clamp(freq_d, BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX);
      int freq = static_cast<int>(std::round(haptics::tools::genericNormalization(
          BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
          freq_d)));
      continuous.addFrequencyAt(freq, it->first - continuous.getPosition());
      // IF NO KEYFRAME AFTER BUT STILL INSIDE, END MODULATION SWITCH BACK TO BASE FREQUENCY
      if (it == frequencies->end() - 1) {
        int freq = static_cast<int>(std::round(haptics::tools::genericNormalization(
            BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
            base_freq)));
        continuous.addFrequencyAt(freq, it->first - continuous.getPosition());
      }
    }
  }
}

} // namespace haptics::encoder
