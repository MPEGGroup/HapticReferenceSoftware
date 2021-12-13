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

#include "../include/AhapEncoder.h"
#include "../../tools/src/Tools.h"
#include <fstream>

const double SEC_TO_MSEC = 1000.0;
const float DEFAULT_AMPLITUDE = 0.5;
const int DEFAULT_FREQUENCY = 90;

const double BASE_AMPLITUDE_MIN = 0;
const double BASE_AMPLITUDE_MAX = 1;
const double ACTUAL_AMPLITUDE_MIN = 0;
const double ACTUAL_AMPLITUDE_MIN_C = 0.6138;
const double ACTUAL_AMPLITUDE_MIN_T = 0.792;

const int BASE_FREQUENCY_MIN = 0;
const int BASE_FREQUENCY_MAX = 1;
const int ACTUAL_FREQUENCY_MIN = 65;
const int ACTUAL_FREQUENCY_MAX = 300;

namespace haptics::encoder {

[[nodiscard]] auto AhapEncoder::encode(std::string& filename) -> int {
  std::ifstream ifs(filename);
  nlohmann::json json = nlohmann::json::parse(ifs);

  std::cout << "get Version : " << json.at("Version") << std::endl;
  std::cout << "get Metadata : " << json.at("Metadata") << std::endl;
  std::cout << "get Pattern : " << json.at("Pattern") << std::endl;

  nlohmann::json pattern = json.at("Pattern");

  std::vector<std::pair<int,double>> amplitudes;
  std::vector<std::pair<int, double>> frequencies;
  std::vector<haptics::types::Effect> continuous;
  std::vector<haptics::types::Effect> transients;

  std::cout << "amplitudes size before" << amplitudes.size() << "\n" << std::endl;

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
  std::cout << "amplitudes size after" << amplitudes.size() << "\n" << std::endl;
  std::cout << "frequencies size after" << frequencies.size() << "\n" << std::endl;

  for (nlohmann::json e : pattern) {
  
    if (e.contains("Event")) {
      if (e.at("Event").at("EventType") == "HapticTransient") {
        ret = extractTransients(&(e.at("Event")), &transients, &amplitudes, &frequencies);
        if (ret != 0) {
          std::cout << "ERROR IN TRANSIENT EXTRACTION" << std::endl;
          return EXIT_FAILURE;
        }
      }

    }
  }

  for (types::Effect t : transients) {
    std::cout << t.getKeyframeAt(0).getAmplitudeModulation() << std::endl;
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
  k.setAmplitudeModulation(DEFAULT_FREQUENCY);
  
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

    if (first_kf_a > amplitudes->begin() && first_kf_a < amplitudes->end()) {
      //MULTIPLY AMPLITUDE MODULATION
      k.setAmplitudeModulation(static_cast<float>(haptics::tools::linearInterpolation(*(first_kf_a - 1), *first_kf_a, t.getPosition())) * k.getAmplitudeModulation());
    } else if(first_kf_a == amplitudes->begin()) {
      k.setAmplitudeModulation(static_cast<float>(amplitudes->front().second) * k.getAmplitudeModulation());
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
      k.setFrequencyModulation(freq + k.getFrequencyModulation());
    } else if(first_kf_f == frequencies->begin()) {
      k.setFrequencyModulation(BASE_FREQUENCY_MAX);
    }
  }

  t.addKeyframe(k);

  transients->push_back(t);

  return EXIT_SUCCESS;
}


} // namespace haptics::encoder