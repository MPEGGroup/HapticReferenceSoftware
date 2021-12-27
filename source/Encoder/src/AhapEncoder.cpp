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
#include <fstream>

const double SEC_TO_MSEC = 1000.0;
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

[[nodiscard]] auto AhapEncoder::encode(std::string& filename) -> int {
  std::ifstream ifs(filename);
  nlohmann::json json = nlohmann::json::parse(ifs);

  nlohmann::json pattern = json.at("Pattern");

  std::vector<std::pair<int,double>> amplitudes;
  std::vector<std::pair<int, double>> frequencies;
  std::vector<haptics::types::Effect> continuous;
  std::vector<haptics::types::Effect> transients;

  int ret = 0;

  //FOR LOOP ON KEYFRAMES
  for (nlohmann::json e : pattern) {
    if (e.contains("ParameterCurve")) {
      if (e.at("ParameterCurve").at("ParameterID") == "HapticIntensityControl") {
        ret = extractKeyframes(&(e.at("ParameterCurve")), &amplitudes);
        if (ret != 0) {
          std::cout << "ERROR IN AMPLITUDE EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }

      if (e.at("ParameterCurve").at("ParameterID") == "HapticSharpnessControl") {
        ret = extractKeyframes(&(e.at("ParameterCurve")), &frequencies);
        if (ret != 0) {
          std::cout << "ERROR IN FREQUENCY EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }

  for (nlohmann::json e : pattern) {
  
    if (e.contains("Event")) {
      if (e.at("Event").at("EventType") == "HapticTransient") {
        ret = extractTransients(&(e.at("Event")), &transients, &amplitudes, &frequencies);
        if (ret != 0) {
          std::cout << "ERROR IN TRANSIENT EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }

      if (e.at("Event").at("EventType") == "HapticContinuous") {
        ret = extractContinuous(&(e.at("Event")), &continuous, &amplitudes, &frequencies);
        if (ret != 0) {
          std::cout << "ERROR IN CONTINUOUS EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}


[[nodiscard]] auto AhapEncoder::extractKeyframes(nlohmann::json *parameterCurve, std::vector<std::pair<int,double>> * keyframes) -> int {
  
  for (nlohmann::json kahap : parameterCurve->at("ParameterCurveControlPoints")) {
    std::pair<int, double> k;
    // TIME + curve offset
    k.first = static_cast<int>((kahap.at("Time").get<double>() + parameterCurve->at("Time").get<double>()) * SEC_TO_MSEC);
    // VALUE
    k.second = kahap.at("ParameterValue").get<double>();

    keyframes->push_back(k);
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto AhapEncoder::extractTransients(nlohmann::json* event, std::vector<haptics::types::Effect>* transients, 
                                                          std::vector<std::pair<int, double>> * amplitudes, 
                                                          std::vector<std::pair<int, double>> * frequencies) -> int {  
  
  haptics::types::Effect t = haptics::types::Effect(static_cast<int>((event->at("Time").get<double>() * SEC_TO_MSEC)), 
                                                    0,
                                                    haptics::types::BaseSignal::Sine);


  haptics::types::Keyframe k;
  k.setAmplitudeModulation(DEFAULT_AMPLITUDE);
  k.setFrequencyModulation(DEFAULT_FREQUENCY);
  
  //SET VALUES AS DEFINED
  for (nlohmann::json param : event->at("EventParameters")) {
    if (param.at("ParameterID") == "HapticIntensity") {
      double amp = tools::genericNormalization(BASE_AMPLITUDE_MIN, BASE_AMPLITUDE_MAX,
                                        ACTUAL_AMPLITUDE_MIN, ACTUAL_AMPLITUDE_MIN_T,
                                        param.at("ParameterValue").get<double>());
      k.setAmplitudeModulation(static_cast<float>(amp));
    }

    if (param.at("ParameterID") == "HapticSharpness") {
      k.setFrequencyModulation(DEFAULT_FREQUENCY);
    }
  }

  //SET MODULATED VALUES IF APPLICABLE
  if (!amplitudes->empty()) {
    // FIND FIRST KEYFRAME AFTER THE EFFECT
    auto first_kf_a = std::find_if(amplitudes->begin(), amplitudes->end(), [t](std::pair<int, double> a) { return a.first >= t.getPosition(); });

    //IF NOT FIRST KEY FRAME && THERE IS A KEYFRAME && KEYFRAME IS NOT THE LAST ONE AND ON THE EVENT BEGINNING
    if (first_kf_a > amplitudes->begin() && first_kf_a < amplitudes->end()) {
      //MULTIPLY AMPLITUDE MODULATION
        k.setAmplitudeModulation(static_cast<float>(haptics::tools::linearInterpolation(*(first_kf_a - 1), *first_kf_a, t.getPosition())) * k.getAmplitudeModulation().value());
    } else if(first_kf_a == amplitudes->begin()) {
      k.setAmplitudeModulation(static_cast<float>(amplitudes->front().second) * k.getAmplitudeModulation().value());
    }
  }

  if (!frequencies->empty()) {
    auto first_kf_f = std::find_if(frequencies->begin(), frequencies->end(), [t](std::pair<int, double> f) { return f.first >= t.getPosition(); });

    if (first_kf_f != frequencies->begin() && first_kf_f < frequencies->end()) {
      //CONVERT FREQUENCY VALUE
      double f = haptics::tools::linearInterpolation(*(first_kf_f - 1), *first_kf_f, t.getPosition());
      int freq = static_cast<int>(haptics::tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX,
                                                  ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX, f));
      // SUM FREQUENCY MODULATION
      k.setFrequencyModulation(freq + k.getFrequencyModulation().value());
    } else if(first_kf_f == frequencies->begin()) {
      k.setFrequencyModulation(ACTUAL_FREQUENCY_MAX);
    }
  }

  t.addKeyframe(k);

  transients->push_back(t);

  return EXIT_SUCCESS;
}


// NOLINTNEXTLINE(readability-function-size, readability-function-cognitive-complexity)
[[nodiscard]] auto AhapEncoder::extractContinuous(nlohmann::json* event,
  std::vector<haptics::types::Effect>* continuous,
  std::vector<std::pair<int, double>>* amplitudes,
  std::vector<std::pair<int, double>>* frequencies) -> int {

  haptics::types::Effect c =
      haptics::types::Effect(static_cast<int>(event->at("Time").get<double>() * SEC_TO_MSEC), 0,
                             haptics::types::BaseSignal::Sine);

  haptics::types::Keyframe k_start;
  haptics::types::Keyframe k_end;
  k_start.setAmplitudeModulation(DEFAULT_AMPLITUDE);
  k_start.setFrequencyModulation(DEFAULT_FREQUENCY);
  k_end.setAmplitudeModulation(DEFAULT_AMPLITUDE);
  k_end.setFrequencyModulation(DEFAULT_FREQUENCY);
  double base_freq = BASE_FREQUENCY_MAX;

  k_end.setRelativePosition(static_cast<int>(event->at("EventDuration").get<double>() * SEC_TO_MSEC));

  // SET VALUES AS DEFINED
  for (nlohmann::json param : event->at("EventParameters")) {
    if (param.at("ParameterID") == "HapticIntensity") {
      double amp = tools::genericNormalization(BASE_AMPLITUDE_MIN, BASE_AMPLITUDE_MAX,
                                               ACTUAL_AMPLITUDE_MIN, ACTUAL_AMPLITUDE_MIN_C,
                                               param.at("ParameterValue").get<double>());
      k_start.setAmplitudeModulation(static_cast<float>(amp));
      k_end.setAmplitudeModulation(k_start.getAmplitudeModulation());
    }

    if (param.at("ParameterID") == "HapticSharpness") {
      base_freq = param.at("ParameterValue").get<double>();
      double freq = tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX,
                                                ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
                                                base_freq);
      k_start.setFrequencyModulation(static_cast<int>(freq));
      k_end.setFrequencyModulation(k_start.getFrequencyModulation());
    }
  }
  c.addKeyframe(k_start);
  
  // SET MODULATED VALUES IF APPLICABLE
  //AMPLITUDE
  if (!amplitudes->empty()) {
    float base_amp = k_start.getAmplitudeModulation().value();
    // FIND FIRST KEYFRAME AFTER THE EFFECT
    auto first_kf_a = std::find_if(amplitudes->begin(), amplitudes->end(), [c](std::pair<int, double> a) { return a.first >= c.getPosition(); });


    //IF FIRST KEYFRAME ON START OF EVENT, UPDATE EVENT
    if (first_kf_a->first == c.getPosition() && first_kf_a != amplitudes->end() -1) {
      c.getKeyframeAt(0).setAmplitudeModulation(c.getKeyframeAt(0).getAmplitudeModulation().value() * static_cast<float>(first_kf_a->second));
      first_kf_a++;
    } else if ((c.getPosition() < first_kf_a->first) &&
               first_kf_a != amplitudes->begin()) {
      float amp = static_cast<float>(haptics::tools::linearInterpolation(
                       *(first_kf_a - 1), *first_kf_a, c.getPosition())) *
                   c.getKeyframeAt(0).getAmplitudeModulation().value();
      c.getKeyframeAt(0).setAmplitudeModulation(amp);
    }

    for (auto it = first_kf_a; it < amplitudes->end(); it++) {
      //IF AFTER OR ON THE END OF THE EFFECT, UPDATE END
      if (it->first >= (c.getPosition() + k_end.getRelativePosition())) {
        float amp = static_cast<float>(haptics::tools::linearInterpolation(
            *(it - 1), *it, c.getPosition() + k_end.getRelativePosition()));
        k_end.setAmplitudeModulation(amp * k_end.getAmplitudeModulation().value());
        break;
      }
      //IF IN THE MIDDLE OF THE EFFECT, ADD KEYFRAME
      if (c.getPosition() < it->first &&
          it->first < (c.getPosition() + k_end.getRelativePosition())) {
        //IF NO KEYFRAME BEFORE, BEGIN MODULATION
        haptics::types::Keyframe ka;
        if (it == amplitudes->begin()) {
          ka.setRelativePosition(it->first - c.getPosition());
          ka.setAmplitudeModulation(base_amp);
          c.addKeyframe(ka);
        }
        ka.setRelativePosition(it->first - c.getPosition());
        ka.setAmplitudeModulation(static_cast<float>(it->second) * base_amp);
        c.addKeyframe(ka);
        // IF NO KEYFRAME AFTER BUT STILL INSIDE, END MODULATION SWITCH BACK TO BASE AMPLITUDE
        if (it == amplitudes->end() - 1) {
          ka.setRelativePosition(it->first - c.getPosition());
          ka.setAmplitudeModulation(base_amp);
          c.addKeyframe(ka);
        }
      }
    }
  }

  //FREQUENCIES
  if (!frequencies->empty()) {
    // FIND FIRST KEYFRAME AFTER THE EFFECT
    auto first_kf_f = std::find_if(frequencies->begin(), frequencies->end(), [c](std::pair<int, double> a) { return a.first >= c.getPosition(); });

    // IF FIRST KEYFRAME ON START OF EVENT, UPDATE EVENT
    if (first_kf_f->first == c.getPosition() && first_kf_f != frequencies->end() - 1) {
      int freq = static_cast<int>(haptics::tools::genericNormalization(
          BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
          first_kf_f->second + base_freq));
      c.getKeyframeAt(0).setFrequencyModulation(std::clamp(freq, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX));
      first_kf_f++;
    } else if ((c.getPosition() < first_kf_f->first) && first_kf_f != frequencies->begin()) {
      double freq = haptics::tools::chirpInterpolation(
          (first_kf_f - 1)->first, (first_kf_f)->first, (first_kf_f - 1)->second + base_freq,
          (first_kf_f)->second + base_freq, c.getPosition());
      freq = std::clamp(freq, BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX);
      c.getKeyframeAt(0).setFrequencyModulation(static_cast<int>(
          haptics::tools::genericNormalization(BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX,
                                               ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX, freq)));
    }

    for (auto it = first_kf_f; it < frequencies->end(); it++) {
      // IF AFTER OR ON THE END OF THE EFFECT, UPDATE END
      if (it->first >= (c.getPosition() + k_end.getRelativePosition())) {
        double freq = haptics::tools::chirpInterpolation(
            (it - 1)->first, it->first, (it - 1)->second + base_freq,
            it->second + base_freq, c.getPosition() + k_end.getRelativePosition());
        freq = std::clamp(freq + base_freq, BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX);
        k_end.setFrequencyModulation(static_cast<int>(haptics::tools::genericNormalization(
            BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
            freq)));
        break;
      }
      // IF IN THE MIDDLE OF THE EFFECT, ADD KEYFRAME
      if (c.getPosition() < it->first &&
          it->first < (c.getPosition() + k_end.getRelativePosition())) {
        // IF NO KEYFRAME BEFORE, BEGIN MODULATION
        haptics::types::Keyframe ka;
        if (it == frequencies->begin()) {
          ka.setRelativePosition(it->first - c.getPosition());
          int freq = static_cast<int>(haptics::tools::genericNormalization(
              BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
              base_freq));
          ka.setFrequencyModulation(freq);
          c.addKeyframe(ka);
        }
        ka.setRelativePosition(it->first - c.getPosition());
        double freq = std::clamp(it->second + base_freq, BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX);
        ka.setFrequencyModulation(static_cast<int>(haptics::tools::genericNormalization(
            BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
            freq)));
        c.addKeyframe(ka);
        // IF NO KEYFRAME AFTER BUT STILL INSIDE, END MODULATION SWITCH BACK TO BASE FREQUENCY
        if (it == frequencies->end() - 1) {
          ka.setRelativePosition(it->first - c.getPosition());
          int freq = static_cast<int>(haptics::tools::genericNormalization(
              BASE_FREQUENCY_MIN, BASE_FREQUENCY_MAX, ACTUAL_FREQUENCY_MIN, ACTUAL_FREQUENCY_MAX,
              base_freq));
          ka.setFrequencyModulation(freq);
          c.addKeyframe(ka);
        }
      }
    }
  }

  c.addKeyframe(k_end);

  continuous->push_back(c);

  return EXIT_SUCCESS;
}

} // namespace haptics::encoder