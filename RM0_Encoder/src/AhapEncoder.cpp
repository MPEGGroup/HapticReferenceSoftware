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
#include <fstream>

namespace haptics::encoder {

[[nodiscard]] auto AhapEncoder::encode(std::string& filename) -> int {
  std::ifstream ifs(filename);
  nlohmann::json json = nlohmann::json::parse(ifs);

  std::cout << "get Version : " << json.at("Version") << std::endl;
  std::cout << "get Metadata : " << json.at("Metadata") << std::endl;
  std::cout << "get Pattern : " << json.at("Pattern") << std::endl;

  nlohmann::json pattern = json.at("Pattern");

  std::vector<std::pair<double,double>> amplitudes;
  std::vector<std::pair<double, double>> frequencies;
  std::vector<haptics::types::Note> continuous;
  std::vector<haptics::types::Note> transient;

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

  return EXIT_SUCCESS;
}


[[nodiscard]] auto AhapEncoder::extractKeyframes(nlohmann::json *parameterCurve, std::vector<std::pair<double,double>> * keyframes) -> int {
  
  for (nlohmann::json kahap : parameterCurve->at("ParameterCurveControlPoints")) {
    std::pair<double, double> k;
    // TIME + curve offset
    k.first = kahap.at("Time").get<double>() + parameterCurve->at("Time").get<double>();
    // VALUE
    k.second = kahap.at("ParameterValue").get<double>();

    keyframes->push_back(k);
  }

  return 0;
}

} // namespace haptics::encoder