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

#include <Types/include/BandType.h>
#include <Types/include/CurveType.h>
#include <Types/include/Keyframe.h>
#include <map>
#include <string>
#include <vector>

using haptics::types::Keyframe;

namespace haptics::types {

enum class BaseSignal { Sine = 0, Square = 1, Triangle = 2, SawToothUp = 3, SawToothDown = 4 };

enum class EffectType { Basis = 0, Reference = 1, Timeline = 2 };

static const std::map<std::string, BaseSignal> stringToBaseSignal = {
    {"Sine", BaseSignal::Sine},
    {"Square", BaseSignal::Square},
    {"Triangle", BaseSignal::Triangle},
    {"SawToothUp", BaseSignal::SawToothUp},
    {"SawToothDown", BaseSignal::SawToothDown}};
static const std::map<BaseSignal, std::string> baseSignalToString = {
    {BaseSignal::Sine, "Sine"},
    {BaseSignal::Square, "Square"},
    {BaseSignal::Triangle, "Triangle"},
    {BaseSignal::SawToothUp, "SawToothUp"},
    {BaseSignal::SawToothDown, "SawToothDown"}};

static const std::map<std::string, EffectType> stringToEffectType = {
    {"Basis", EffectType::Basis},
    {"Reference", EffectType::Reference},
    {"Timeline", EffectType::Timeline}};

static const std::map<EffectType, std::string> effectTypeToString = {
    {EffectType::Basis, "Basis"},
    {EffectType::Reference, "Reference"},
    {EffectType::Timeline, "Timeline"}};

class Effect {
public:
  explicit Effect() = default;
  explicit Effect(int newPosition, float newPhase, BaseSignal newBaseSignal,
                  EffectType newEffectType)
      : position(newPosition)
      , phase(newPhase)
      , keyframes({})
      , baseSignal(newBaseSignal)
      , effectType(newEffectType){};

  [[nodiscard]] auto getId() const -> int;
  auto setId(int newId) -> void;
  [[nodiscard]] auto getPosition() const -> int;
  auto setPosition(int newPosition) -> void;
  [[nodiscard]] auto getPhase() const -> float;

  auto setPhase(float newPhase) -> void;
  [[nodiscard]] auto getBaseSignal() const -> BaseSignal;
  auto setBaseSignal(BaseSignal newBaseSignal) -> void;
  [[nodiscard]] auto getEffectType() const -> EffectType;
  auto setEffectType(EffectType newEffectType) -> void;
  auto getKeyframesSize() -> size_t;
  auto getKeyframeAt(int index) -> Keyframe &;
  auto replaceKeyframeAt(int index, types::Keyframe &newKeyframe) -> bool;
  auto removeKeyframeAt(int index) -> bool;
  auto addKeyframe(Keyframe &newKeyframe) -> void;
  auto addKeyframe(std::optional<int> position, std::optional<double> amplitudeModulation,
                   std::optional<int> frequencyModulation) -> void;
  auto addAmplitudeAt(std::optional<float> amplitude, int position) -> bool;
  auto addAmplitudeAt(std::optional<float> amplitude, int position, bool overrideIfAlreadyExists)
      -> bool;
  auto addFrequencyAt(std::optional<int> frequency, int position) -> bool;
  auto getEffectTimeLength(types::BandType bandType, double transientDuration) -> double;

  auto getTimelineSize() -> size_t;
  auto getTimelineEffectAt(int index) -> Effect &;
  auto addTimelineEffect(Effect &newEffect) -> void;

  auto isEquivalent(Effect &effect) -> bool;
  // Use Absolute position not relative
  auto EvaluateVectorial(double position, int lowFrequencyLimit, int highFrequencyLimit) -> double;
  auto EvaluateWavelet(double position, double windowLength) -> double;
  auto EvaluateTransient(double position, double transientDuration) -> double;
  auto EvaluateKeyframes(double position, types::CurveType curveType) -> double;

private:
  int id = -1;
  int position = 0;
  float phase = 0;
  std::vector<Keyframe> keyframes = std::vector<Keyframe>{};
  BaseSignal baseSignal = BaseSignal::Sine;
  EffectType effectType = EffectType::Basis;
  std::vector<Effect> timeline = std::vector<Effect>{};

  [[nodiscard]] auto computeBaseSignal(double time, double frequency, double phase) const -> double;
};
} // namespace haptics::types

#endif // EFFECT_H
