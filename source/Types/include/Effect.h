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

#ifndef EFFECT_H
#define EFFECT_H

#include <Types/include/EncodingModality.h>
#include <Types/include/BandType.h>
#include <Types/include/Keyframe.h>
#include <vector>

using haptics::types::Keyframe;

namespace haptics::types {

enum class BaseSignal {
  Sine = 0,
  Square = Sine + 1,
  Triangle = Square + 1,
  SawToothUp = Triangle + 1,
  SawToothDown = SawToothUp + 1
};

class Effect {
public:
  explicit Effect() = default;
  explicit Effect(int newPosition, float newPhase, BaseSignal newBaseSignal)
      : position(newPosition)
      , phase(newPhase)
      , keyframes({})
      , baseSignal(newBaseSignal) {};

  [[nodiscard]] auto getPosition() const -> int;
  auto setPosition(int newPosition) -> void;
  [[nodiscard]] auto getPhase() const -> float;

  auto setPhase(float newPhase) -> void;
  [[nodiscard]] auto getBaseSignal() const -> BaseSignal;
  auto setBaseSignal(BaseSignal newBaseSignal) -> void;
  auto getKeyframesSize() -> size_t;
  auto getKeyframeAt(int index) -> Keyframe&;
  auto addKeyframe(Keyframe &newKeyframe) -> void;
  auto addKeyframe(std::optional<int> position, std::optional<double> amplitudeModulation,
                   std::optional<int> frequencyModulation) -> void;
  auto addAmplitudeAt(float amplitude, int position) -> bool;
  auto addFrequencyAt(int frequency, int position) -> bool;
  auto Effect::getEffectTimeLength(types::BandType bandType,
                                   types::EncodingModality encodingModality, int windowLength,
                                   double transientDuration) -> double;
  //Use Absolute position not relative
  auto EvaluateVectorial(double position, int lowFrequencyLimit, int highFrequencyLimit) -> double;
  auto EvaluateQuantized(double position, double windowLength) -> double;
  auto EvaluateTransient(double position, double transientDuration) -> double;
  auto EvaluateKeyframes(double position) -> double;

private:
  int position = 0;
  float phase = 0;
  std::vector<Keyframe> keyframes = std::vector<Keyframe>{};
  BaseSignal baseSignal = BaseSignal::Sine;
  [[nodiscard]] auto computeBaseSignal(double time, double frequency) const -> double;
};
} // namespace haptics::types

#endif //EFFECT_H
