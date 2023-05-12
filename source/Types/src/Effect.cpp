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

#include <Tools/include/Tools.h>
#include <Types/include/Effect.h>
#include <algorithm>
#include <cmath>
#include <functional>

namespace haptics::types {

[[nodiscard]] auto Effect::getId() const -> int { return id; }
auto Effect::setId(int newId) -> void { id = newId; }

[[nodiscard]] auto Effect::getPosition() const -> int { return position; }

auto Effect::setPosition(int newPosition) -> void { position = newPosition; }

[[nodiscard]] auto Effect::getSemantic() const -> std::optional<std::string> { return semantic; }

auto Effect::setSemantic(std::string &newSemantic) -> void { semantic = newSemantic; }

[[nodiscard]] auto Effect::getPhase() const -> float { return phase; }

auto Effect::setPhase(float newPhase) -> void { phase = newPhase; }

[[nodiscard]] auto Effect::getBaseSignal() const -> BaseSignal { return baseSignal; }

auto Effect::setBaseSignal(BaseSignal newBaseSignal) -> void { baseSignal = newBaseSignal; }

[[nodiscard]] auto Effect::getEffectType() const -> EffectType { return effectType; }

auto Effect::setEffectType(EffectType newEffectType) -> void { effectType = newEffectType; }

auto Effect::getKeyframesSize() -> size_t { return keyframes.size(); }

auto Effect::getKeyframeAt(int index) -> haptics::types::Keyframe & { return keyframes.at(index); }

auto Effect::replaceKeyframeAt(int index, types::Keyframe &newKeyframe) -> bool {
  if (index < 0 || index >= static_cast<int>(keyframes.size())) {
    return false;
  }

  this->keyframes[index] = newKeyframe;
  return true;
}

auto Effect::removeKeyframeAt(int index) -> bool {
  if (index < 0 || index >= static_cast<int>(keyframes.size())) {
    return false;
  }

  this->keyframes.erase(this->keyframes.begin() + index);
  return true;
}

auto Effect::addKeyframe(haptics::types::Keyframe &newKeyframe) -> void {
  keyframes.push_back(newKeyframe);
}

auto Effect::addAmplitudeAt(std::optional<float> amplitude, int position) -> bool {
  return this->addAmplitudeAt(amplitude, position, true);
}

auto Effect::addAmplitudeAt(std::optional<float> amplitude, int position,
                            bool overrideIfAlreadyExists) -> bool {
  auto kit =
      std::find_if(keyframes.begin(), keyframes.end(), [position](haptics::types::Keyframe k) {
        return k.getRelativePosition() >= position;
      });

  if (kit == keyframes.end()) {
    Keyframe kf = Keyframe(position, amplitude, std::nullopt);
    keyframes.push_back(kf);
    return true;
  }

  if ((kit)->getRelativePosition() == position) {
    if (overrideIfAlreadyExists) {
      (kit)->setAmplitudeModulation(amplitude);
    } else {
      Keyframe kf = Keyframe(position, amplitude, std::nullopt);
      keyframes.insert(kit + 1, kf);
    }
  } else {
    Keyframe kf = Keyframe(position, amplitude, std::nullopt);
    keyframes.insert(kit, kf);
  }

  return true;
}

auto Effect::addFrequencyAt(std::optional<int> frequency, int position) -> bool {
  auto kit =
      std::find_if(keyframes.begin(), keyframes.end(), [position](haptics::types::Keyframe k) {
        return k.getRelativePosition() >= position;
      });

  if (kit == keyframes.end()) {
    Keyframe kf = Keyframe(position, std::nullopt, frequency);
    keyframes.push_back(kf);
    return true;
  }

  if (kit->getRelativePosition() == position) {
    kit->setFrequencyModulation(frequency);
  } else {
    Keyframe kf = Keyframe(position, std::nullopt, frequency);
    keyframes.insert(kit, kf);
  }

  return true;
}

auto Effect::addKeyframe(std::optional<int> position, std::optional<double> amplitudeModulation,
                         std::optional<int> frequencyModulation) -> void {
  Keyframe newKf(position, amplitudeModulation, frequencyModulation);
  this->addKeyframe(newKf);
}

auto Effect::isEquivalent(Effect &effect) -> bool {
  if (effectType != EffectType::Basis || effect.effectType != EffectType::Basis ||
      phase != effect.getPhase() || baseSignal != effect.getBaseSignal()) {
    return false;
  }

  auto size = keyframes.size();
  if (size != effect.getKeyframesSize()) {
    return false;
  }

  for (uint32_t i = 0; i < size; i++) {
    if (getKeyframeAt((int)i) != effect.getKeyframeAt((int)i)) {
      return false;
    }
  }
  return true;
}

auto Effect::EvaluateVectorial(double position, int lowFrequencyLimit, int highFrequencyLimit)
    -> double {
  double res = 0;

  double max_position = this->position + this->getEffectTimeLength(BandType::VectorialWave, 0);

  if (position < this->position || position > max_position || keyframes.empty()) {
    return res;
  }

  double relativePosition = position - this->position;

  // AMPLITUDE MODULATION
  double amp_modulation = 1;
  // First amplitude keyframe after the relative position
  auto firstAmplitudeKeyframeAfterPositionIt = std::find_if(
      keyframes.begin(), keyframes.end(), [relativePosition](haptics::types::Keyframe k) {
        return k.getAmplitudeModulation().has_value() && k.getRelativePosition().has_value() &&
               k.getRelativePosition().value() >= relativePosition;
      });
  if (firstAmplitudeKeyframeAfterPositionIt < keyframes.end()) {
    // First amplitude keyframe before the keyframe previously found
    auto firstAmplitudeKeyframeBeforePositionIt = std::find_if(
        std::make_reverse_iterator(firstAmplitudeKeyframeAfterPositionIt), keyframes.rend(),
        [](haptics::types::Keyframe k) { return k.getAmplitudeModulation().has_value(); });
    if (firstAmplitudeKeyframeBeforePositionIt == keyframes.rend()) {
      amp_modulation = firstAmplitudeKeyframeAfterPositionIt->getAmplitudeModulation().value();
    } else {
      float a0 = firstAmplitudeKeyframeBeforePositionIt->getAmplitudeModulation().value();
      int t0 = firstAmplitudeKeyframeBeforePositionIt->getRelativePosition().has_value()
                   ? firstAmplitudeKeyframeBeforePositionIt->getRelativePosition().value()
                   : 0;
      float a1 = firstAmplitudeKeyframeAfterPositionIt->getAmplitudeModulation().value();
      int t1 = firstAmplitudeKeyframeAfterPositionIt->getRelativePosition().value();
      amp_modulation = haptics::tools::linearInterpolation({t0, a0}, {t1, a1}, relativePosition);
    }
  } else {
    auto amplitudeKeyframeIt =
        std::find_if(keyframes.rbegin(), keyframes.rend(), [](haptics::types::Keyframe k) {
          return k.getAmplitudeModulation().has_value();
        });
    if (amplitudeKeyframeIt < keyframes.rend()) {
      amp_modulation = amplitudeKeyframeIt->getAmplitudeModulation().value();
    }
  }

  // FREQUENCY MODULATION
  double freq_modulation = 0;
  double phi = this->getPhase();
  // First frequency keyframe after the relative position
  // Find phase corresponding to this keyframe
  auto firstFrequencyKeyframeAfterPositionIt = keyframes.begin();
  auto firstFrequencyKeyframeBeforePositionIt = keyframes.rend();
  while (firstFrequencyKeyframeAfterPositionIt < keyframes.end()) {
    if (firstFrequencyKeyframeAfterPositionIt->getFrequencyModulation().has_value()) {
      int pos = firstFrequencyKeyframeAfterPositionIt->getRelativePosition().has_value()
                    ? firstFrequencyKeyframeAfterPositionIt->getRelativePosition().value()
                    : 0;
      firstFrequencyKeyframeBeforePositionIt = std::find_if(
          std::make_reverse_iterator(firstFrequencyKeyframeAfterPositionIt), keyframes.rend(),
          [](haptics::types::Keyframe k) { return k.getFrequencyModulation().has_value(); });
      if (pos >= relativePosition) {
        break;
      }
      if (firstFrequencyKeyframeBeforePositionIt < keyframes.rend()) {
        int deltaT =
            pos - (firstFrequencyKeyframeBeforePositionIt->getRelativePosition().has_value()
                       ? firstFrequencyKeyframeBeforePositionIt->getRelativePosition().value()
                       : 0);
        phi += M_PI * deltaT * MS_2_S *
               (firstFrequencyKeyframeBeforePositionIt->getFrequencyModulation().value() +
                firstFrequencyKeyframeAfterPositionIt->getFrequencyModulation().value());
      } else if (pos > 0) { // first keyframe with frequency value
        phi += 2 * M_PI * pos * MS_2_S *
               firstFrequencyKeyframeAfterPositionIt->getFrequencyModulation().value();
      }
    }

    firstFrequencyKeyframeAfterPositionIt++;
  }

  if (firstFrequencyKeyframeAfterPositionIt < keyframes.end()) {
    if (firstFrequencyKeyframeBeforePositionIt == keyframes.rend()) {
      freq_modulation = static_cast<double>(
          firstFrequencyKeyframeAfterPositionIt->getFrequencyModulation().value());
    } else {
      double f0 = static_cast<double>(
          firstFrequencyKeyframeBeforePositionIt->getFrequencyModulation().value());
      int t0 = firstFrequencyKeyframeBeforePositionIt->getRelativePosition().has_value()
                   ? firstFrequencyKeyframeBeforePositionIt->getRelativePosition().value()
                   : 0;
      auto f1 = static_cast<double>(
          firstFrequencyKeyframeAfterPositionIt->getFrequencyModulation().value());
      int t1 = firstFrequencyKeyframeAfterPositionIt->getRelativePosition().has_value()
                   ? firstFrequencyKeyframeAfterPositionIt->getRelativePosition().value()
                   : 0;

      freq_modulation = tools::chirpInterpolation(t0, t1, f0, f1, relativePosition);
      freq_modulation = std::clamp(freq_modulation, static_cast<double>(lowFrequencyLimit),
                                   static_cast<double>(highFrequencyLimit));
      // To replace the evaluated relative position in the range [0; t1-t0], this will prevent
      // unexpected behaviours on the chirp evaluation
      relativePosition -= t0;
    }
  } else {
    auto frequencyKeyframeIt =
        std::find_if(keyframes.rbegin(), keyframes.rend(), [](haptics::types::Keyframe k) {
          return k.getFrequencyModulation().has_value();
        });
    if (frequencyKeyframeIt < keyframes.rend()) {
      freq_modulation = frequencyKeyframeIt->getFrequencyModulation().value();
    }
  }

  return amp_modulation * this->computeBaseSignal(MS_2_S * relativePosition, freq_modulation, phi);
}

auto Effect::EvaluateWavelet(double position, double windowLength) -> double {
  double relativePosition = position - this->getPosition();
  int index = std::floor(relativePosition / windowLength * (double)this->getKeyframesSize());

  if (index >= (int)this->getKeyframesSize()) {
    return 0;
  }
  auto myKeyframe = keyframes.begin() + index;
  if (!myKeyframe->getAmplitudeModulation().has_value()) {
    return 0;
  }

  return myKeyframe->getAmplitudeModulation().value();
}

auto Effect::EvaluateTransient(double position, double transientDuration) -> double {
  const double relativePosition = position - this->getPosition();

  auto checkingFunction = [&](Keyframe kf) {
    return kf.getRelativePosition().has_value() &&
           kf.getRelativePosition().value() <= relativePosition &&
           kf.getRelativePosition().value() + transientDuration >= relativePosition;
  };
  auto it = std::find_if(keyframes.rbegin(), keyframes.rend(), checkingFunction);

  double res = 0;
  while (it != keyframes.rend() && checkingFunction(*it)) {
    if (!it->getAmplitudeModulation().has_value()) {
      continue;
    }

    res += std::sin(4 * M_PI * (relativePosition - it->getRelativePosition().value()) /
                    transientDuration) *
           it->getAmplitudeModulation().value();
    it++;
  }

  return res;
}

auto Effect::EvaluateKeyframes(double position, types::CurveType curveType) -> double {
  const double relativePosition = position - this->getPosition();
  double res = 0;
  auto k_after = std::find_if(
      keyframes.begin(), keyframes.end(), [relativePosition](haptics::types::Keyframe k) {
        return k.getRelativePosition().has_value() && k.getAmplitudeModulation().has_value() &&
               k.getRelativePosition() > relativePosition;
      });

  if (k_after < keyframes.end()) {
    // first KF before position
    auto k_before = std::find_if(
        std::make_reverse_iterator(k_after), keyframes.rend(), [](haptics::types::Keyframe k) {
          return k.getRelativePosition().has_value() && k.getAmplitudeModulation().has_value();
        });
    if (k_before == keyframes.rend()) {
      return k_after->getAmplitudeModulation().value();
    }

    double t0 = MS_2_S * k_before->getRelativePosition().value();
    double f0 = k_before->getAmplitudeModulation().value();
    double t1 = MS_2_S * k_after->getRelativePosition().value();
    double f1 = k_after->getAmplitudeModulation().value();

    double t = MS_2_S * relativePosition;
    switch (curveType) {
    case types::CurveType::Cubic: {
      double h = t1 - t0;
      return f0 + (f1 - f0) * (3 * h + 2 * (t0 - t)) * std::pow(t - t0, 2) / std::pow(h, 3);
    }
    case types::CurveType::Linear:
      return (f0 * (t1 - t) + f1 * (t - t0)) / (t1 - t0);
    default:
      return 0;
    }
  }

  return res;
}

[[nodiscard]] auto Effect::computeBaseSignal(double time, double frequency, double phase) const
    -> double {
  const double half = .5;
  const double quarter = .25;

  if (frequency != 0) {
    time += phase / (2 * M_PI * frequency);
  }
  switch (this->getBaseSignal()) {
  case BaseSignal::Sine:
    return std::sin(2 * M_PI * time * frequency);
  case BaseSignal::Square:
    return 1 - 2 * std::round(time * frequency - std::floor(time * frequency));
  case BaseSignal::Triangle:
    return 1 - 4 * std::abs(std::round(time * frequency - quarter) - (time * frequency - quarter));
  case BaseSignal::SawToothUp:
    return 2 * (time * frequency - std::floor(time * frequency + half));
  case BaseSignal::SawToothDown:
    return 2 * (std::floor(time * frequency + half) - time * frequency);
  default:
    return 1;
  }
}

auto Effect::getEffectTimeLength(types::BandType bandType, double transientDuration) -> double {
  if (this->getKeyframesSize() == 0) {
    return 0;
  }

  auto lastKeyframe =
      std::find_if(keyframes.rbegin(), keyframes.rend(),
                   [](haptics::types::Keyframe k) { return k.getRelativePosition().has_value(); });
  switch (bandType) {
  case types::BandType::Transient:
    return lastKeyframe->getRelativePosition().has_value()
               ? (lastKeyframe->getRelativePosition().value() + transientDuration)
               : 0;
  case types::BandType::Curve:
  case types::BandType::VectorialWave:
    return lastKeyframe->getRelativePosition().has_value()
               ? lastKeyframe->getRelativePosition().value()
               : 0;
  default:
    break;
  }
  return 0;
}

auto Effect::getTimelineSize() -> size_t { return timeline.size(); }
auto Effect::getTimelineEffectAt(int index) -> haptics::types::Effect & {
  return timeline.at(index);
}
auto Effect::addTimelineEffect(Effect &newEffect) -> void { timeline.push_back(newEffect); }

} // namespace haptics::types
