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

[[nodiscard]] auto Effect::getPosition() const -> int { return position; }

auto Effect::setPosition(int newPosition) -> void { position = newPosition; }

[[nodiscard]] auto Effect::getPhase() const -> float { return phase; }

auto Effect::setPhase(float newPhase) -> void { phase = newPhase; }

[[nodiscard]] auto Effect::getBaseSignal() const -> BaseSignal { return baseSignal; }

auto Effect::setBaseSignal(BaseSignal newBaseSignal) -> void { baseSignal = newBaseSignal; }

auto Effect::getKeyframesSize() -> size_t { return keyframes.size(); }

auto Effect::getKeyframeAt(int index) -> haptics::types::Keyframe & { return keyframes.at(index); }

auto Effect::replaceKeyframeAt(int index, types::Keyframe &newKeyframe) -> bool {
  if (index < 0 || index >= static_cast<int>(keyframes.size())) {
    return false;
  }

  this->keyframes[index] = newKeyframe;
  return true;
}

auto Effect::addKeyframe(haptics::types::Keyframe &newKeyframe) -> void {
  keyframes.push_back(newKeyframe);
}

auto Effect::addAmplitudeAt(float amplitude, int position) -> bool {
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
    (kit)->setAmplitudeModulation(amplitude);
  } else {
    Keyframe kf = Keyframe(position, amplitude, std::nullopt);
    keyframes.insert(kit, kf);
  }

  return true;
}

auto Effect::addFrequencyAt(int frequency, int position) -> bool {
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
  this->addKeyframe(*(new Keyframe(position, amplitudeModulation, frequencyModulation)));
}

// NOLINTNEXTLINE(readability-function-size)
auto Effect::EvaluateVectorial(double position, int lowFrequencyLimit, int highFrequencyLimit)
    -> double {
  double res = 0;

  if (position < this->position ||
      position > this->position +
                     this->getEffectTimeLength(BandType::Wave, EncodingModality::Vectorial, 0, 0)) {
    return res;
  }

  if (keyframes.empty()) {
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
      freq_modulation = static_cast<double>(firstFrequencyKeyframeAfterPositionIt->getFrequencyModulation().value());
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

auto Effect::EvaluateQuantized(double position, double windowLength) -> double {
  double relativePosition = position - this->getPosition();
  int index = std::floor(relativePosition / windowLength);
  if (index >= (int)this->getKeyframesSize()) {
    return 0;
  }

  auto myKeyframe = keyframes.begin() + index;
  if (!myKeyframe->getAmplitudeModulation().has_value() ||
      !myKeyframe->getFrequencyModulation().has_value()) {
    return 0;
  }

  double t = MS_2_S * relativePosition;
  return std::sin(t * myKeyframe->getFrequencyModulation().value() * 2 * M_PI + this->getPhase()) *
         myKeyframe->getAmplitudeModulation().value();
}

auto Effect::EvaluateWavelet(double position, double windowLength) -> double {
  double relativePosition = position - this->getPosition();
  int index = std::floor(relativePosition / windowLength * (double)this->getKeyframesSize());

  // std::cout << "windowLength: " << windowLength << std::endl;
  // std::cout << "EvaluateWavelet position: " << position << std::endl;
  // std::cout << "relativePosition: " << relativePosition << std::endl;
  // std::cout << "EvaluateWavelet index: " << index << std::endl;

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

auto Effect::EvaluateKeyframes(double position) -> double {
  double res = 0;
  auto k_after =
      std::find_if(keyframes.begin(), keyframes.end(), [position](haptics::types::Keyframe k) {
        return k.getRelativePosition().has_value() && k.getAmplitudeModulation().has_value() &&
               k.getRelativePosition() > position;
      });

  if (k_after < keyframes.end()) {
    // first KF before position
    auto k_before = keyframes.begin();
    bool found = true;
    for (auto it = k_after - 1; it > keyframes.begin(); it--) {
      if (it->getRelativePosition().has_value() && it->getAmplitudeModulation().has_value()) {
        k_before = it;
        break;
      }
      if (it == keyframes.begin()) {
        found = false;
      }
    }
    if (!found) {
      return k_after->getAmplitudeModulation().value();
    }

    double x0 = MS_2_S * k_before->getRelativePosition().value();
    double f0 = k_before->getAmplitudeModulation().value();
    double t0 = 0;
    double x1 = MS_2_S * k_after->getRelativePosition().value();
    double f1 = k_after->getAmplitudeModulation().value();
    double t1 = 0;

    double t = MS_2_S * position;

    double c2 = 0;
    double c3 = 0;
    double df = 0;
    double h = 0;
    h = x1 - x0;
    df = (f1 - f0) / h;

    c2 = -(2 * t0 - 3 * df + t1) / h;
    c3 = (t0 - 2 * df + t1) / h / h;

    auto result = static_cast<float>(f0 + (t - x0) * (t0 + (t - x0) * (c2 + (t - x0) * c3)));
    return std::max(std::min(1.0F, result), -1.0F);
  }

  return res;
}

[[nodiscard]] auto Effect::computeBaseSignal(double time, double frequency, double phase) const -> double {
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

auto Effect::getEffectTimeLength(types::BandType bandType, types::EncodingModality encodingModality,
                                 int windowLength, double transientDuration) -> double {
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
    return lastKeyframe->getRelativePosition().has_value()
               ? lastKeyframe->getRelativePosition().value()
               : 0;
  case types::BandType::Wave:
    switch (encodingModality) {
    case types::EncodingModality::Quantized:
      return static_cast<int>(this->getKeyframesSize()) * windowLength;
    case types::EncodingModality::Vectorial:
      return lastKeyframe->getRelativePosition().has_value()
                 ? lastKeyframe->getRelativePosition().value()
                 : 0;
    default:
      break;
    }
  default:
    break;
  }
  return 0;
}

} // namespace haptics::types