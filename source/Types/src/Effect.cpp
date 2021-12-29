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
#include <Types/include/Effect.h>
#include <Tools/include/Tools.h>
#include <cmath>

namespace haptics::types {

[[nodiscard]] auto Effect::getPosition() const -> int {
  return position;
}

auto Effect::setPosition(int newPosition) -> void {
  position = newPosition;
}

[[nodiscard]] auto Effect::getPhase() const -> float {
  return phase;
}

auto Effect::setPhase(float newPhase) -> void {
  phase = newPhase;
}

[[nodiscard]] auto Effect::getBaseSignal() const -> BaseSignal {
  return baseSignal;
}

auto Effect::setBaseSignal(BaseSignal newBaseSignal) -> void {
  baseSignal = newBaseSignal;
}

auto Effect::getKeyframesSize() -> size_t {
  return keyframes.size();
}

auto Effect::getKeyframeAt(int index) -> haptics::types::Keyframe& {
  return keyframes.at(index);
}

auto Effect::addKeyframe(haptics::types::Keyframe& newKeyframe) -> void {
  keyframes.push_back(newKeyframe);
}

auto Effect::addKeyframe(int position, std::optional<double> amplitudeModulation, std::optional<int> frequencyModulation) -> void {
  this->addKeyframe(*(new Keyframe(position, amplitudeModulation, frequencyModulation)));
}

auto Effect::EvaluateVectorial(int position) -> double {
  double res = 0;

  if (position < this->position) {
    return res;
  }

  if (keyframes.empty()) {
    return res;
  }

  int relativePosition = position - this->position;

  //res = linearAmplitudeModulation blabla
  double amp_modulation = 1;
  
  //First KF AFTER
  auto k_a_after = std::find_if(keyframes.begin(), keyframes.end(),
                                    [relativePosition](haptics::types::Keyframe k) { 
                                      return k.getAmplitudeModulation().has_value() && k.getRelativePosition() >= relativePosition; 
                                    }); 


  //IF AMPLITUDE MODULATION (SHOULD BE BUT TO BE SURE)
  if (k_a_after < keyframes.end()) {
    // first KF before position
    auto k_a_before = keyframes.begin();
    for (auto it = k_a_after - 1; it >= keyframes.begin(); it--) {
      if (it->getAmplitudeModulation().has_value()) {
        k_a_before = it;
        break;
      }
    }

    amp_modulation = haptics::tools::linearInterpolation(
        {k_a_before->getRelativePosition(), k_a_before->getAmplitudeModulation().value()},
        {k_a_after->getRelativePosition(), k_a_after->getAmplitudeModulation().value()}, relativePosition);
  }

  // IF FREQUENCY MODULATION (SHOULD BE BUT TO BE SURE)

  double freq_modulation = 1;
  // first KF after position
  auto k_f_after = std::find_if(keyframes.begin(), keyframes.end(),
                                [relativePosition](haptics::types::Keyframe k) {
                                  return k.getFrequencyModulation().has_value() &&
                                         k.getRelativePosition() >= relativePosition;
                                });
  if (k_f_after < keyframes.end()) {
    // first KF before position
    auto k_f_before = keyframes.begin();
    for (auto it = k_f_after - 1; it >= keyframes.begin(); it--) {
      if (it->getFrequencyModulation().has_value()) {
        k_f_before = it;
        break;
      }
    }

    // Modulation
    int f0 = k_f_before->getFrequencyModulation().value();
    int f1 = k_f_after->getFrequencyModulation().value();
    int t = relativePosition - k_f_before->getRelativePosition();
    float DeltaT = k_f_after->getRelativePosition() - k_f_before->getRelativePosition();

    freq_modulation = (M_PI * (f0 * t + 0.5 * t * t * (f1 - f0) / DeltaT));
    //TODO CHANGE BASE SIGNAL
    freq_modulation = std::sin(freq_modulation);
  }
  //-----
  
  return amp_modulation * freq_modulation;
}

auto Effect::EvaluateQuantized(int position) -> double {
  double res = 0;

  //freq = first KF before position
  //amp = first KF before position
  //res = sine(freq,amp)

  int relativePosition = position - this->position;

  auto k_before = keyframes.begin();

  for (auto it = keyframes.end(); it >= keyframes.begin(); it--) {
    if (it->getRelativePosition() <= relativePosition) {
      k_before = it;
      break;
    }
  }



  return res;
}

auto Effect::EvaluateTransient(int position) -> double {
  double res = 0;

  //?

  return res;
}

auto Effect::EvaluateKeyframes(int position) -> double {
  double res = 0;

  float x0 = m_keyFrames[_KeyframeBeforePointIndex].m_time;
  float f0 = m_keyFrames[_KeyframeBeforePointIndex].m_value;
  float t0 = m_keyFrames[_KeyframeBeforePointIndex].m_outSlope; // *m_Curve[k - 1].outWeight;
  float x1 = m_keyFrames[_KeyframeAfterPointIndex].m_time;
  float f1 = m_keyFrames[_KeyframeAfterPointIndex].m_value;
  float t1 = m_keyFrames[_KeyframeAfterPointIndex].m_inSlope; // *m_Curve[k].inWeight;

  double c2;
  double c3;
  double df;
  double h;
  h = x1 - x0;
  df = (f1 - f0) / h;

  c2 = -(2.0 * t0 - 3.0 * df + t1) / h;
  c3 = (t0 - 2.0 * df + t1) / h / h;

  float result = f0 + (_position - x0) * (t0 + (_position - x0) * (c2 + (_position - x0) * c3));
  return std::max(std::min(1.0f, result), -1.0f);

  return res;
}

} // namespace haptics::types