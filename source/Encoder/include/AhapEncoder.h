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

#ifndef AHAPENCODER_H
#define AHAPENCODER_H

#include <Types/include/Band.h>
#include <Types/include/Effect.h>
#include <Types/include/Keyframe.h>
#include <Types/include/Perception.h>
#include <iostream>
#include <nlohmann/json.hpp>

namespace haptics::encoder {

class AhapEncoder {
public:
  [[nodiscard]] auto static encode(std::string &filename, types::Perception &out) -> int;
  [[nodiscard]] auto static extractTransients(
      nlohmann::json *event, std::vector<haptics::types::Effect> *transients,
      const std::vector<std::pair<int, double>> *amplitudes,
      const std::vector<std::pair<int, double>> *frequencies) -> int;
  [[nodiscard]] auto static extractContinuous(
      nlohmann::json *event, std::vector<haptics::types::Effect> *continuous,
      const std::vector<std::pair<int, double>> *amplitudes,
      const std::vector<std::pair<int, double>> *frequencies) -> int;
  [[nodiscard]] auto static extractKeyframes(nlohmann::json *parameterCurve,
                                             std::vector<std::pair<int, double>> *keyframes) -> int;

private:
  [[nodiscard]] auto static modulateContinuousOnAmplitude(
      const std::vector<std::pair<int, double>> *amplitudes, types::Effect &continuous,
      const Keyframe &firstKeyframe, Keyframe &lastKeyframe) -> void;
  [[nodiscard]] auto static modulateContinuousOnFrequency(
      const std::vector<std::pair<int, double>> *frequencies, types::Effect &continuous,
      Keyframe &lastKeyframe, double base_freq) -> void;
  static const int MIN_AHAP_FREQUENCY = 65;
  static const int MAX_AHAP_FREQUENCY = 300;

};
} // namespace haptics::encoder
#endif // AHAPENCODER_H